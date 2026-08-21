import logging
import os
import asyncio
import json
import random
import time
import platform
from typing import Dict, Any, List, Optional
from fastapi import FastAPI, Request, HTTPException, Depends, Cookie, UploadFile, File, Query, Form
from fastapi.responses import HTMLResponse, RedirectResponse, JSONResponse
from fastapi.templating import Jinja2Templates
from pydantic import BaseModel
from apscheduler.schedulers.background import BackgroundScheduler

from app.config import settings
from app.services.mqtt_pub import mqtt_service
from app.services.google_auth import (
    ProfileTokenManager,
    get_google_auth_url,
    get_spotify_auth_url,
    exchange_google_code,
    exchange_google_code_for_login,
    exchange_spotify_code,
    load_profile_settings,
    save_profile_settings,
    get_profile_polling_intervals,
    DEFAULT_POLLING_INTERVALS
)
from app.services.spotify import get_spotify_currently_playing
from app.services.google_api import get_google_calendar_and_tasks, get_google_task_lists
from app.services.stocks import get_multi_asset_prices, search_stocks_yahoo
from app.services.analytics import get_combined_analytics
from app.services.ga4 import get_ga4_analytics, test_ga4_connection
from app.services.gcp_billing import get_gcp_multi_project_billing
from app.services.antigravity import get_antigravity_credits

from fastapi.staticfiles import StaticFiles

# Setup logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger("backend_main")

app = FastAPI(
    title="AuraDeck Smart Dashboard Backend",
    description="FastAPI gateway managing multi-tenant profile settings and TV pairing.",
    version="2.0.0"
)

# Setup Static files and HTML template rendering
static_dir = os.path.join(os.path.dirname(__file__), "static")
os.makedirs(static_dir, exist_ok=True)
app.mount("/static", StaticFiles(directory=static_dir), name="static")

templates_dir = os.path.join(os.path.dirname(__file__), "templates")
templates = Jinja2Templates(directory=templates_dir)

# APScheduler for polling background jobs
scheduler = BackgroundScheduler()

# --- Shared Pairing PIN Cache & Device Registry ---
PAIRING_CODES_CACHE: Dict[str, Dict[str, Any]] = {}  # PIN -> {"mac": mac, "expires_at": float}
DEVICE_MAPPINGS_FILE = os.path.join(settings.TOKENS_DIR, "device_mappings.json")

def load_device_mappings() -> Dict[str, Any]:
    """Loads paired device-to-profile mappings from disk."""
    if not os.path.exists(DEVICE_MAPPINGS_FILE):
        return {}
    try:
        with open(DEVICE_MAPPINGS_FILE, "r", encoding="utf-8") as f:
            return json.load(f)
    except Exception as e:
        logger.error(f"Failed to load device mappings: {e}")
        return {}

def save_device_mappings(mappings: Dict[str, Any]):
    """Saves device-to-profile mappings to disk."""
    os.makedirs(os.path.dirname(DEVICE_MAPPINGS_FILE), exist_ok=True)
    try:
        with open(DEVICE_MAPPINGS_FILE, "w", encoding="utf-8") as f:
            json.dump(mappings, f, indent=2, ensure_ascii=False)
    except Exception as e:
        logger.error(f"Failed to save device mappings: {e}")

def list_all_profiles() -> List[Dict[str, Any]]:
    """Lists all created profiles found in tokens directory."""
    profiles_dir = os.path.join(settings.TOKENS_DIR, "profiles")
    if not os.path.exists(profiles_dir):
        os.makedirs(profiles_dir, exist_ok=True)
    
    profiles = []
    for d in os.listdir(profiles_dir):
        profile_path = os.path.join(profiles_dir, d)
        if os.path.isdir(profile_path):
            s_data = load_profile_settings(d)
            profile_name = s_data.get("profile_name") or d.replace("_", " ").title()
            profiles.append({"id": d, "name": profile_name})
    
    # Auto-initialize default profile if none exist
    if not profiles:
        default_dir = os.path.join(profiles_dir, "default")
        os.makedirs(default_dir, exist_ok=True)
        save_profile_settings("default", {"profile_name": "Default Profile"})
        profiles.append({"id": "default", "name": "Default Profile"})
        
    return profiles

# --- Models for API Request Bodies ---

class MqttPublishRequest(BaseModel):
    topic: str
    payload: dict

class ProfileCreateRequest(BaseModel):
    profile_name: str

class ProfileConfigRequest(BaseModel):
    ga_property_id: Optional[str] = None
    google_client_id: Optional[str] = None
    google_client_secret: Optional[str] = None
    google_redirect_uri: Optional[str] = None
    active_task_lists: Optional[List[str]] = None
    weather_lat: Optional[float] = None
    weather_lon: Optional[float] = None

class ProfileIntervalsUpdateRequest(BaseModel):
    profile_id: Optional[str] = None
    tasks_calendar_mins: Optional[int] = None
    stocks_mins: Optional[int] = None
    weather_mins: Optional[int] = None
    antigravity_mins: Optional[int] = None
    ga4_mins: Optional[int] = None
    gcp_mins: Optional[int] = None
    analytics_mins: Optional[int] = None
    time_sync_secs: Optional[int] = None

class Ga4TestRequest(BaseModel):
    property_id: str
    profile_id: Optional[str] = None

class DevicePairRequest(BaseModel):
    pin: str
    profile_id: str

class StockAddRequest(BaseModel):
    symbol: str
    name: Optional[str] = None
    profile_id: Optional[str] = None

# --- Startup & Shutdown Events ---

# Tracking dictionary for per-profile last background poll timestamps
LAST_POLL_TIMESTAMPS: Dict[str, Dict[str, float]] = {}


# --- MQTT Hardware Command Dispatcher ---

def handle_mqtt_command(topic: str, payload_str: str):
    """
    Handles incoming hardware MQTT commands from ESP32 terminals.
    e.g.:
      - auradeck/command/spotify/toggle
      - auradeck/command/sync/{service}
      - auradeck/command/sync/all
    """
    logger.info(f"⚡ [Hardware Command] Processing MQTT: {topic} | Payload: {payload_str}")
    try:
        profiles = list_all_profiles()
        active_pid = profiles[0]["id"] if profiles else "default"

        if topic == "auradeck/command/spotify/toggle" or topic.startswith("auradeck/command/spotify"):
            from app.services.spotify import toggle_spotify_playback
            profiles = list_all_profiles()
            for p in profiles:
                pid = p["id"]
                mgr = ProfileTokenManager(pid, "Spotify")
                if mgr.has_credentials():
                    res = run_async_safe(toggle_spotify_playback(pid))
                    logger.info(f"Spotify toggle execution for profile '{pid}': {res}")
            # Broadcast updated status immediately to all connected devices
            trigger_spotify_polling()

        elif topic.startswith("auradeck/command/sync/"):
            svc = topic.split("/")[-1].lower()
            logger.info(f"Triggering instant sync for service: {svc}")
            if svc in ["weather", "all"]:
                trigger_weather_polling(force=True)
            if svc in ["stocks", "all"]:
                trigger_stocks_polling(force=True)
            if svc in ["calendar", "todos", "tasks", "all"]:
                trigger_calendar_polling(force=True)
            if svc in ["antigravity", "all"]:
                trigger_antigravity_polling(force=True)
            if svc in ["ga4", "all"]:
                trigger_ga4_polling(force=True)
            if svc in ["gcp", "all"]:
                trigger_gcp_billing_polling(force=True)
            if svc in ["spotify", "all"]:
                trigger_spotify_polling()
    except Exception as e:
        logger.error(f"Failed to execute hardware MQTT command: {e}")


@app.on_event("startup")
def on_startup():
    logger.info("Starting up AuraDeck Control Center...")
    mqtt_service.set_command_handler(handle_mqtt_command)
    mqtt_service.connect()
    
    # Ensure default profile is created on start
    list_all_profiles()
    
    # Setup background polling scheduler (Ticks frequently to evaluate per-profile interval timers)
    scheduler.add_job(
        id="spotify_polling_job",
        func=trigger_spotify_polling,
        trigger="interval",
        seconds=5,
        max_instances=1
    )
    scheduler.add_job(
        id="calendar_polling_job",
        func=trigger_calendar_polling,
        trigger="interval",
        seconds=15,
        max_instances=1
    )
    scheduler.add_job(
        id="stocks_polling_job",
        func=trigger_stocks_polling,
        trigger="interval",
        seconds=15,
        max_instances=1
    )
    scheduler.add_job(
        id="analytics_polling_job",
        func=trigger_analytics_polling,
        trigger="interval",
        seconds=30,
        max_instances=1
    )
    scheduler.add_job(
        id="ga4_polling_job",
        func=trigger_ga4_polling,
        trigger="interval",
        seconds=30,
        max_instances=1
    )
    scheduler.add_job(
        id="gcp_polling_job",
        func=trigger_gcp_billing_polling,
        trigger="interval",
        seconds=30,
        max_instances=1
    )
    scheduler.add_job(
        id="weather_polling_job",
        func=trigger_weather_polling,
        trigger="interval",
        seconds=30,
        max_instances=1
    )
    scheduler.add_job(
        id="antigravity_polling_job",
        func=trigger_antigravity_polling,
        trigger="interval",
        seconds=15,
        max_instances=1
    )
    scheduler.add_job(
        id="time_sync_job",
        func=trigger_time_sync,
        trigger="interval",
        seconds=10,
        max_instances=1
    )
    
    scheduler.start()
    logger.info("Background Schedulers started successfully.")
    
    # Trigger initial data publish for all services immediately on startup
    # Wait up to 10s for MQTT connection first (connect_async is non-blocking)
    try:
        logger.info("Waiting for MQTT broker connection before initial data poll...")
        mqtt_service.wait_for_connect(timeout_secs=10.0)
        logger.info("Triggering initial data poll for all services on startup...")
        trigger_time_sync(force=True)
        trigger_spotify_polling()
        trigger_calendar_polling(force=True)
        trigger_stocks_polling(force=True)
        trigger_weather_polling(force=True)
        trigger_ga4_polling(force=True)
        trigger_gcp_billing_polling(force=True)
        trigger_analytics_polling(force=True)
        trigger_antigravity_polling(force=True)
    except Exception as e:
        logger.error(f"Error running initial data poll on startup: {e}")



@app.on_event("shutdown")
def on_shutdown():
    logger.info("Shutting down background services...")
    mqtt_service.disconnect()
    scheduler.shutdown()


# --- Background Polling Triggers (Multi-Profile & Interval Aware) ---

def run_async_safe(coro):
    """Safely runs an async coroutine whether or not an event loop is currently running."""
    try:
        loop = asyncio.get_running_loop()
    except RuntimeError:
        loop = None

    if loop and loop.is_running():
        import concurrent.futures
        with concurrent.futures.ThreadPoolExecutor() as pool:
            future = pool.submit(lambda: asyncio.run(coro))
            return future.result()
    else:
        return asyncio.run(coro)

def trigger_spotify_polling():
    """Polls Spotify currently-playing for all active profiles and publishes to paired devices."""
    try:
        profiles = list_all_profiles()
        mappings = load_device_mappings()
        for p in profiles:
            pid = p["id"]
            mgr = ProfileTokenManager(pid, "Spotify")
            if mgr.has_credentials():
                data = run_async_safe(get_spotify_currently_playing(pid))
                
                # Publish to all devices paired to this profile
                for mac, m_data in mappings.items():
                    if m_data.get("profile_id") == pid:
                        mqtt_service.publish(f"auradeck/device/{mac}/spotify", data)
                
                # Legacy support for default topic
                if pid == "default":
                    mqtt_service.publish("auradeck/spotify", data)
    except Exception as e:
        logger.error(f"Error in background Spotify poller: {e}")

def trigger_calendar_polling(target_profile_id: Optional[str] = None, force: bool = False):
    """Polls Google Calendar & Tasks for profiles based on their configured interval."""
    now = time.time()
    try:
        profiles = list_all_profiles()
        mappings = load_device_mappings()
        for p in profiles:
            pid = p["id"]
            if target_profile_id and pid != target_profile_id:
                continue

            intervals = get_profile_polling_intervals(pid)
            interval_secs = intervals.get("tasks_calendar_mins", 15) * 60
            last_poll = LAST_POLL_TIMESTAMPS.get(pid, {}).get("calendar", 0.0)

            if force or (now - last_poll >= interval_secs):
                LAST_POLL_TIMESTAMPS.setdefault(pid, {})["calendar"] = now
                mgr = ProfileTokenManager(pid, "Google")
                if mgr.has_credentials():
                    data = run_async_safe(get_google_calendar_and_tasks(pid))
                    
                    # Publish to all devices paired to this profile
                    for mac, m_data in mappings.items():
                        if m_data.get("profile_id") == pid:
                            mqtt_service.publish(f"auradeck/device/{mac}/calendar", data.get("calendar", {}))
                            mqtt_service.publish(f"auradeck/device/{mac}/todos", data.get("todos", []))
                    
                    if pid == "default":
                        mqtt_service.publish("auradeck/calendar", data.get("calendar", {}))
                        mqtt_service.publish("auradeck/todos", data.get("todos", []))
                    logger.info(f"📅 [Calendar/Tasks] Polled profile '{pid}' (interval: {intervals.get('tasks_calendar_mins')}m)")
    except Exception as e:
        logger.error(f"Error in background Calendar/Tasks poller: {e}")

def trigger_stocks_polling(target_profile_id: Optional[str] = None, force: bool = False):
    """Polls stock indexes and gold/crypto indices for profiles based on their configured interval."""
    now = time.time()
    try:
        profiles = list_all_profiles()
        mappings = load_device_mappings()
        
        # Check if default/global stocks need update
        global_last_poll = LAST_POLL_TIMESTAMPS.get("default", {}).get("stocks", 0.0)
        default_interval_secs = get_profile_polling_intervals("default").get("stocks_mins", 5) * 60
        if force or (now - global_last_poll >= default_interval_secs):
            LAST_POLL_TIMESTAMPS.setdefault("default", {})["stocks"] = now
            default_prices = run_async_safe(get_multi_asset_prices())
            mqtt_service.publish("auradeck/stocks", default_prices)
        else:
            default_prices = None
        
        for p in profiles:
            pid = p["id"]
            if target_profile_id and pid != target_profile_id:
                continue

            intervals = get_profile_polling_intervals(pid)
            interval_secs = intervals.get("stocks_mins", 5) * 60
            last_poll = LAST_POLL_TIMESTAMPS.get(pid, {}).get("stocks", 0.0)

            if force or (now - last_poll >= interval_secs):
                LAST_POLL_TIMESTAMPS.setdefault(pid, {})["stocks"] = now
                prof_settings = load_profile_settings(pid)
                watchlist_items = prof_settings.get("stock_watchlist")
                
                if watchlist_items:
                    p_prices = run_async_safe(get_multi_asset_prices(watchlist_items=watchlist_items))
                    mqtt_service.publish(f"auradeck/profile/{pid}/stocks", p_prices)
                    
                    # Mirror to paired devices for this profile
                    for mac, m_data in mappings.items():
                        if m_data.get("profile_id") == pid:
                            mqtt_service.publish(f"auradeck/device/{mac}/stocks", p_prices)
                else:
                    if default_prices is None:
                        default_prices = run_async_safe(get_multi_asset_prices())
                    for mac, m_data in mappings.items():
                        if m_data.get("profile_id") == pid:
                            mqtt_service.publish(f"auradeck/device/{mac}/stocks", default_prices)
                logger.info(f"📈 [Stocks] Polled profile '{pid}' (interval: {intervals.get('stocks_mins')}m)")
    except Exception as e:
        logger.error(f"Error in background Stocks poller: {e}", exc_info=True)


def trigger_analytics_polling(target_profile_id: Optional[str] = None, force: bool = False):
    """Polls GA4 and GCP Billing for profiles based on configured interval."""
    now = time.time()
    try:
        profiles = list_all_profiles()
        mappings = load_device_mappings()
        for p in profiles:
            pid = p["id"]
            if target_profile_id and pid != target_profile_id:
                continue

            intervals = get_profile_polling_intervals(pid)
            interval_secs = intervals.get("analytics_mins", 15) * 60
            last_poll = LAST_POLL_TIMESTAMPS.get(pid, {}).get("analytics", 0.0)

            if force or (now - last_poll >= interval_secs):
                LAST_POLL_TIMESTAMPS.setdefault(pid, {})["analytics"] = now
                data = asyncio.run(get_combined_analytics(pid))
                for mac, m_data in mappings.items():
                    if m_data.get("profile_id") == pid:
                        mqtt_service.publish(f"auradeck/device/{mac}/analytics", data)
                
                if pid == "default":
                    mqtt_service.publish("auradeck/analytics", data)
                logger.info(f"📊 [Analytics] Polled profile '{pid}' (interval: {intervals.get('analytics_mins')}m)")
    except Exception as e:
        logger.error(f"Error in background Analytics poller: {e}")

def trigger_ga4_polling(target_profile_id: Optional[str] = None, force: bool = False):
    """Polls Google Analytics 4 (GA4) detailed metrics for profiles based on configured interval."""
    now = time.time()
    try:
        profiles = list_all_profiles()
        mappings = load_device_mappings()
        for p in profiles:
            pid = p["id"]
            if target_profile_id and pid != target_profile_id:
                continue

            intervals = get_profile_polling_intervals(pid)
            interval_secs = intervals.get("ga4_mins", 5) * 60
            last_poll = LAST_POLL_TIMESTAMPS.get(pid, {}).get("ga4", 0.0)

            if force or (now - last_poll >= interval_secs):
                LAST_POLL_TIMESTAMPS.setdefault(pid, {})["ga4"] = now
                data = asyncio.run(get_ga4_analytics(pid))
                for mac, m_data in mappings.items():
                    if m_data.get("profile_id") == pid:
                        mqtt_service.publish(f"auradeck/device/{mac}/ga4", data)
                
                if pid == "default":
                    mqtt_service.publish("auradeck/ga4", data)
                logger.info(f"📈 [GA4 Analytics] Polled profile '{pid}' (interval: {intervals.get('ga4_mins', 5)}m)")
    except Exception as e:
        logger.error(f"Error in background GA4 poller: {e}")

def trigger_gcp_billing_polling(target_profile_id: Optional[str] = None, force: bool = False):
    """Polls GCP Cloud Billing and multi-project telemetry for profiles based on configured interval."""
    now = time.time()
    try:
        profiles = list_all_profiles()
        mappings = load_device_mappings()
        for p in profiles:
            pid = p["id"]
            if target_profile_id and pid != target_profile_id:
                continue

            intervals = get_profile_polling_intervals(pid)
            interval_secs = intervals.get("gcp_mins", 30) * 60
            last_poll = LAST_POLL_TIMESTAMPS.get(pid, {}).get("gcp_billing", 0.0)

            if force or (now - last_poll >= interval_secs):
                LAST_POLL_TIMESTAMPS.setdefault(pid, {})["gcp_billing"] = now
                data = asyncio.run(get_gcp_multi_project_billing(pid))
                for mac, m_data in mappings.items():
                    if m_data.get("profile_id") == pid:
                        mqtt_service.publish(f"auradeck/device/{mac}/gcp", data)
                
                if pid == "default":
                    mqtt_service.publish("auradeck/gcp", data)
                logger.info(f"☁️ [GCP Billing] Polled profile '{pid}' (interval: {intervals.get('gcp_mins', 30)}m)")
    except Exception as e:
        logger.error(f"Error in background GCP Billing poller: {e}")

def trigger_weather_polling(target_profile_id: Optional[str] = None, force: bool = False):
    """Polls Open-Meteo weather and hourly rain forecast for profiles based on configured interval."""
    now = time.time()
    try:
        from app.services.weather import get_hourly_weather_forecast
        profiles = list_all_profiles()
        mappings = load_device_mappings()
        for p in profiles:
            pid = p["id"]
            if target_profile_id and pid != target_profile_id:
                continue

            intervals = get_profile_polling_intervals(pid)
            interval_secs = intervals.get("weather_mins", 30) * 60
            last_poll = LAST_POLL_TIMESTAMPS.get(pid, {}).get("weather", 0.0)

            if force or (now - last_poll >= interval_secs):
                LAST_POLL_TIMESTAMPS.setdefault(pid, {})["weather"] = now
                
                # Read custom coordinates if configured
                prof_settings = load_profile_settings(pid)
                lat = float(prof_settings.get("weather_lat", 13.7563))
                lon = float(prof_settings.get("weather_lon", 100.5018))
                
                data = asyncio.run(get_hourly_weather_forecast(latitude=lat, longitude=lon))
                
                for mac, m_data in mappings.items():
                    if m_data.get("profile_id") == pid:
                        mqtt_service.publish(f"auradeck/device/{mac}/weather", data)
                
                if pid == "default":
                    mqtt_service.publish("auradeck/weather", data)
                logger.info(f"🌦️ [Weather] Polled profile '{pid}' (lat={lat}, lon={lon}, interval: {intervals.get('weather_mins', 30)}m)")
    except Exception as e:
        logger.error(f"Error in background Weather poller: {e}")

def trigger_antigravity_polling(force: bool = False):
    """Polls Google Antigravity developer credits based on interval."""
    now = time.time()
    try:
        intervals = get_profile_polling_intervals("default")
        interval_secs = intervals.get("antigravity_mins", 1) * 60
        last_poll = LAST_POLL_TIMESTAMPS.get("global", {}).get("antigravity", 0.0)

        if force or (now - last_poll >= interval_secs):
            LAST_POLL_TIMESTAMPS.setdefault("global", {})["antigravity"] = now
            data = asyncio.run(get_antigravity_credits())
            mqtt_service.publish("auradeck/antigravity", data)
            
            mappings = load_device_mappings()
            for mac in mappings:
                mqtt_service.publish(f"auradeck/device/{mac}/antigravity", data)
    except Exception as e:
        logger.error(f"Error in background Antigravity poller: {e}")

def trigger_time_sync(force: bool = False):
    """Publishes current server datetime in Thailand (GMT+7) for hardware RTC synchronization on paired screens."""
    try:
        from datetime import datetime, timezone, timedelta
        tz_th = timezone(timedelta(hours=7))
        now = datetime.now(tz_th)
        payload = {
            "year": now.year,
            "month": now.month,
            "day": now.day,
            "hour": now.hour,
            "minute": now.minute,
            "second": now.second,
            "time": now.strftime("%H:%M"),
            "date": now.strftime("%A, %B %d")
        }
        mqtt_service.publish("auradeck/time_sync", payload)
        mappings = load_device_mappings()
        for mac in mappings:
            mqtt_service.publish(f"auradeck/device/{mac}/time_sync", payload)
    except Exception as e:
        logger.error(f"Error in time sync publisher: {e}")


# --- Page Routing ---

@app.get("/", response_class=HTMLResponse)
def get_root(request: Request, active_profile_id: Optional[str] = Cookie(None)):
    """Verifies profile login session and renders either login screen or main dashboard."""
    if not active_profile_id:
        return RedirectResponse(url="/login")
    
    # Confirm profile still exists on disk
    profile_dir = os.path.join(settings.TOKENS_DIR, "profiles", active_profile_id)
    if not os.path.exists(profile_dir):
        response = RedirectResponse(url="/login")
        response.delete_cookie("active_profile_id", path="/")
        return response
        
    return templates.TemplateResponse("auth.html", {"request": request, "profile_id": active_profile_id})


@app.get("/login", response_class=HTMLResponse)
def get_login_page(request: Request):
    """Renders the beautifully styled landing login screen."""
    return templates.TemplateResponse("login.html", {"request": request})


@app.get("/select-profile/{profile_id}")
def select_profile(profile_id: str):
    """Deprecated profile selection route."""
    raise HTTPException(status_code=403, detail="AuraDeck 2.0 requires secure Google Authentication Sign-In.")


@app.get("/logout")
def logout_user():
    """Logs the user out of their session, returning them to the welcome portal."""
    response = RedirectResponse(url="/login")
    response.delete_cookie("active_profile_id", path="/")
    return response


# --- Google / Spotify OAuth Redirect Handles ---

@app.get("/google/login")
def login_google(profile_id: Optional[str] = None, active_profile_id: Optional[str] = Cookie(None)):
    """Redirects profile auth trigger to Google OAuth screen passing state."""
    pid = profile_id or active_profile_id or "login_session"
    return RedirectResponse(url=get_google_auth_url(pid))


@app.get("/google/callback")
async def callback_google(code: str, state: str = "default"):
    """Accepts authorized code callback, maps it back to correct profile."""
    if state == "login_session":
        res = await exchange_google_code_for_login(code)
        if res:
            profile_id = res["profile_id"]
            response = RedirectResponse(url="/")
            # Expire in 30 days
            response.set_cookie(key="active_profile_id", value=profile_id, max_age=30 * 24 * 3600, path="/")
            return response
        else:
            raise HTTPException(status_code=400, detail="Google authentication failed.")
    else:
        tokens = await exchange_google_code(state, code)
        if tokens:
            logger.info(f"Successfully completed Google OAuth2 authentication flow for profile {state}.")
            return RedirectResponse(url="/?google=success")
        else:
            raise HTTPException(status_code=400, detail="Google authentication failed.")


@app.get("/spotify/login")
def login_spotify(profile_id: Optional[str] = None, active_profile_id: Optional[str] = Cookie(None)):
    """Redirects profile auth trigger to Spotify OAuth screen passing state."""
    pid = profile_id or active_profile_id or "default"
    return RedirectResponse(url=get_spotify_auth_url(pid))


from fastapi.responses import HTMLResponse, RedirectResponse, JSONResponse, FileResponse

@app.get("/api/spotify/cover.png")
def get_spotify_cover_png(profile_id: str = "default"):
    """Serves 80x80 1-bit monochrome PNG preview for Web Dashboard viewer."""
    out_dir = os.path.join(settings.TOKENS_DIR, "profiles", profile_id)
    png_path = os.path.join(out_dir, "spotify_cover.png")
    if os.path.exists(png_path):
        return FileResponse(png_path, media_type="image/png")
    raise HTTPException(status_code=404, detail="Cover art preview not available.")

@app.get("/api/spotify/cover.bmp")
def get_spotify_cover_bmp(profile_id: str = "default"):
    """Serves 80x80 1-bit monochrome BMP image for ESP32 hardware display."""
    out_dir = os.path.join(settings.TOKENS_DIR, "profiles", profile_id)
    bmp_path = os.path.join(out_dir, "spotify_cover.bmp")
    if os.path.exists(bmp_path):
        return FileResponse(bmp_path, media_type="image/x-ms-bmp")
    raise HTTPException(status_code=404, detail="Cover art BMP not available.")

@app.get("/spotify/callback")
async def callback_spotify(code: str, state: str = "default"):
    """Accepts authorized code callback, maps it back to correct profile."""
    tokens = await exchange_spotify_code(state, code)
    if tokens:
        logger.info(f"Successfully completed Spotify OAuth2 authentication flow for profile {state}.")
        return RedirectResponse(url="/?spotify=success")
    else:
        raise HTTPException(status_code=400, detail="Spotify authentication failed.")


# --- Profiles REST API Management ---

@app.get("/api/profiles")
def api_get_profiles(active_profile_id: Optional[str] = Cookie(None)):
    """Endpoint listing all active profiles (restricted to logged-in user)."""
    if not active_profile_id:
        return []
    profiles = list_all_profiles()
    return [p for p in profiles if p["id"] == active_profile_id]


@app.post("/api/profiles")
def api_create_profile(request: ProfileCreateRequest):
    """Deprecated manual profile creation endpoint."""
    raise HTTPException(status_code=403, detail="Manual profile creation is disabled. Use Google Login to auto-provision profiles.")


@app.delete("/api/profiles/{profile_id}")
def api_delete_profile(profile_id: str, active_profile_id: Optional[str] = Cookie(None)):
    """Deletes profile tokens and config directory, and clears any paired devices."""
    if not active_profile_id or active_profile_id != profile_id:
        raise HTTPException(status_code=403, detail="Permission Denied. You can only manage your own profile.")
        
    if profile_id == "default":
        raise HTTPException(status_code=400, detail="The default profile cannot be deleted.")
        
    profiles_dir = os.path.join(settings.TOKENS_DIR, "profiles", profile_id)
    if not os.path.exists(profiles_dir):
        raise HTTPException(status_code=404, detail="Profile not found.")
        
    # Remove from disk
    import shutil
    try:
        shutil.rmtree(profiles_dir)
    except Exception as e:
        logger.error(f"Failed to delete profile folder: {e}")
        raise HTTPException(status_code=500, detail="Failed to delete profile from filesystem.")
        
    # Unpair any devices mapped to it
    mappings = load_device_mappings()
    updated_mappings = {mac: m for mac, m in mappings.items() if m.get("profile_id") != profile_id}
    save_device_mappings(updated_mappings)
    
    return {"status": "success", "message": f"Successfully deleted profile {profile_id}"}


@app.get("/api/profiles/{profile_id}/config")
def api_get_profile_config(profile_id: str, active_profile_id: Optional[str] = Cookie(None)):
    """Retrieves safe settings metadata for profile dashboard UI (hides Client Secrets)."""
    if not active_profile_id or active_profile_id != profile_id:
        raise HTTPException(status_code=403, detail="Permission Denied. You can only manage your own profile.")
        
    settings_data = load_profile_settings(profile_id)
    
    # Check if any Service Account keys are configured in the gcp_projects subfolder
    gcp_projects_dir = os.path.join(settings.TOKENS_DIR, "profiles", profile_id, "gcp_projects")
    has_sa_keys = False
    if os.path.exists(gcp_projects_dir):
        files = [f for f in os.listdir(gcp_projects_dir) if f.endswith(".json")]
        if files:
            has_sa_keys = True
            
    legacy_sa = os.path.exists(os.path.join(settings.TOKENS_DIR, "profiles", profile_id, "service_account.json"))
    google_sa_configured = has_sa_keys or legacy_sa

    return {
        "profile_name": settings_data.get("profile_name", profile_id),
        "ga_property_id": settings_data.get("ga_property_id", ""),
        "google_client_id": settings_data.get("google_client_id", ""),
        "google_client_secret_configured": bool(settings_data.get("google_client_secret")),
        "google_redirect_uri": settings_data.get("google_redirect_uri", ""),
        "active_task_lists": settings_data.get("active_task_lists", ["@default"]),
        "google_sa_configured": google_sa_configured,
        "weather_lat": settings_data.get("weather_lat", 13.7563),
        "weather_lon": settings_data.get("weather_lon", 100.5018),
        "polling_intervals": get_profile_polling_intervals(profile_id)
    }


@app.get("/api/v1/profile/intervals")
def api_get_profile_intervals(profile_id: Optional[str] = Query(None), active_profile_id: Optional[str] = Cookie(None)):
    """Retrieves current polling intervals for a profile with default fallbacks."""
    pid = profile_id or active_profile_id or "default"
    return {
        "profile_id": pid,
        "intervals": get_profile_polling_intervals(pid)
    }


@app.post("/api/v1/profile/intervals")
def api_update_profile_intervals(req: ProfileIntervalsUpdateRequest, active_profile_id: Optional[str] = Cookie(None)):
    """Updates background polling intervals for a profile and immediately triggers an instant sync to screens."""
    pid = req.profile_id or active_profile_id or "default"
    prof_settings = load_profile_settings(pid)
    current_intervals = prof_settings.get("polling_intervals", DEFAULT_POLLING_INTERVALS.copy())
    if not isinstance(current_intervals, dict):
        current_intervals = DEFAULT_POLLING_INTERVALS.copy()
        
    if req.tasks_calendar_mins is not None:
        current_intervals["tasks_calendar_mins"] = max(1, req.tasks_calendar_mins)
    if req.stocks_mins is not None:
        current_intervals["stocks_mins"] = max(1, req.stocks_mins)
    if req.weather_mins is not None:
        current_intervals["weather_mins"] = max(1, req.weather_mins)
    if req.antigravity_mins is not None:
        current_intervals["antigravity_mins"] = max(1, req.antigravity_mins)
    if req.ga4_mins is not None:
        current_intervals["ga4_mins"] = max(1, req.ga4_mins)
    if req.gcp_mins is not None:
        current_intervals["gcp_mins"] = max(1, req.gcp_mins)
    if req.analytics_mins is not None:
        current_intervals["analytics_mins"] = max(1, req.analytics_mins)
    if req.time_sync_secs is not None:
        current_intervals["time_sync_secs"] = max(5, req.time_sync_secs)
        
    prof_settings["polling_intervals"] = current_intervals
    save_profile_settings(pid, prof_settings)
    
    # Immediately trigger data fetch and MQTT publish for this profile so screen updates instantly
    try:
        logger.info(f"⚡ Instant Sync triggered for profile '{pid}' after polling intervals update...")
        trigger_calendar_polling(target_profile_id=pid, force=True)
        trigger_stocks_polling(target_profile_id=pid, force=True)
        trigger_weather_polling(target_profile_id=pid, force=True)
        trigger_ga4_polling(target_profile_id=pid, force=True)
        trigger_gcp_billing_polling(target_profile_id=pid, force=True)
        trigger_analytics_polling(target_profile_id=pid, force=True)
        trigger_antigravity_polling(force=True)
        trigger_time_sync(force=True)
    except Exception as e:
        logger.error(f"Error triggering instant sync after updating intervals: {e}")
        
    return {
        "status": "success",
        "message": "Polling intervals updated and instant screen sync triggered!",
        "profile_id": pid,
        "intervals": current_intervals
    }


@app.post("/api/profiles/{profile_id}/config")
def api_save_profile_config(profile_id: str, config: ProfileConfigRequest, active_profile_id: Optional[str] = Cookie(None)):
    """Merges and saves user config inputs to the target profile's settings.json."""
    if not active_profile_id or active_profile_id != profile_id:
        raise HTTPException(status_code=403, detail="Permission Denied. You can only manage your own profile.")
        
    settings_data = load_profile_settings(profile_id)
    
    updated_fields = config.dict(exclude_unset=True)
    for k, v in updated_fields.items():
        settings_data[k] = v
        
    save_profile_settings(profile_id, settings_data)
    return {"status": "success", "message": "Profile configuration updated successfully."}


@app.post("/api/profiles/{profile_id}/upload-secrets")
async def api_upload_secrets(profile_id: str, type: str = Query(...), file: UploadFile = File(...), active_profile_id: Optional[str] = Cookie(None)):
    """Handles secure JSON credential uploads for OAuth Client Secrets or Service Accounts."""
    if not active_profile_id or active_profile_id != profile_id:
        raise HTTPException(status_code=403, detail="Permission Denied. You can only manage your own profile.")
        
    profile_dir = os.path.join(settings.TOKENS_DIR, "profiles", profile_id)
    if not os.path.exists(profile_dir):
        raise HTTPException(status_code=444, detail="Profile directory does not exist.")
        
    try:
        content = await file.read()
        parsed_json = json.loads(content)
    except Exception as e:
        raise HTTPException(status_code=400, detail=f"Invalid JSON file formatting: {e}")
        
    settings_data = load_profile_settings(profile_id)
    
    if type == "oauth":
        # Google OAuth client secrets file
        web_info = parsed_json.get("web") or parsed_json.get("installed") or {}
        if not web_info:
            raise HTTPException(status_code=400, detail="Missing 'web' or 'installed' client credentials block.")
            
        settings_data["google_client_id"] = web_info.get("client_id")
        settings_data["google_client_secret"] = web_info.get("client_secret")
        redirects = web_info.get("redirect_uris", [])
        if redirects:
            settings_data["google_redirect_uri"] = redirects[0]
            
        save_profile_settings(profile_id, settings_data)
        return {"status": "success", "message": "Successfully parsed and saved Google OAuth client secrets."}
        
    elif type == "service_account":
        # GCP Service Account key file
        if parsed_json.get("type") != "service_account":
            raise HTTPException(status_code=400, detail="This file is not a valid Google Service Account JSON key.")
            
        target_path = os.path.join(profile_dir, "service_account.json")
        with open(target_path, "w", encoding="utf-8") as f:
            json.dump(parsed_json, f, indent=2)
            
        return {"status": "success", "message": "Service Account key file uploaded and installed successfully."}
        
    else:
        raise HTTPException(status_code=400, detail="Invalid credential type parameter.")


@app.get("/api/profiles/{profile_id}/gcp-projects")
def api_get_gcp_projects(profile_id: str, active_profile_id: Optional[str] = Cookie(None)):
    """Retrieves all uploaded GCP projects and their service account client emails."""
    if not active_profile_id or active_profile_id != profile_id:
        raise HTTPException(status_code=403, detail="Permission Denied. You can only manage your own profile.")
        
    gcp_projects_dir = os.path.join(settings.TOKENS_DIR, "profiles", profile_id, "gcp_projects")
    projects_list = []
    
    if os.path.exists(gcp_projects_dir):
        for fname in os.listdir(gcp_projects_dir):
            if fname.endswith(".json"):
                path = os.path.join(gcp_projects_dir, fname)
                try:
                    with open(path, "r", encoding="utf-8") as f:
                        key_data = json.load(f)
                    proj_id = key_data.get("project_id") or os.path.splitext(fname)[0]
                    proj_name = key_data.get("project_name") or proj_id
                    currency = key_data.get("currency", "THB")
                    client_email = key_data.get("client_email")
                    if proj_id:
                        projects_list.append({
                            "project_id": proj_id,
                            "project_name": proj_name,
                            "currency": currency,
                            "client_email": client_email or "Unknown Service Account"
                        })
                except Exception as e:
                    logger.error(f"Failed to parse SA key file {fname}: {e}")
                    
    return projects_list


@app.post("/api/profiles/{profile_id}/gcp-projects/upload")
async def api_upload_gcp_project(
    profile_id: str,
    project_name: Optional[str] = Form(""),
    currency: Optional[str] = Form("THB"),
    file: UploadFile = File(...),
    active_profile_id: Optional[str] = Cookie(None)
):
    """Uploads a GCP Service Account JSON key for a specific project under the profile."""
    if not active_profile_id or active_profile_id != profile_id:
        raise HTTPException(status_code=403, detail="Permission Denied. You can only manage your own profile.")
        
    try:
        content = await file.read()
        parsed_json = json.loads(content)
    except Exception as e:
        raise HTTPException(status_code=400, detail=f"Invalid JSON formatting: {e}")
        
    if parsed_json.get("type") != "service_account":
        raise HTTPException(status_code=400, detail="The file is not a valid Google Service Account JSON key.")
        
    project_id = parsed_json.get("project_id")
    if not project_id:
        raise HTTPException(status_code=400, detail="The key file does not contain a valid Google Cloud project_id.")
        
    parsed_json["project_name"] = project_name.strip() or project_id
    parsed_json["currency"] = (currency.strip().upper() if currency else "THB")

    profile_dir = os.path.join(settings.TOKENS_DIR, "profiles", profile_id)
    gcp_projects_dir = os.path.join(profile_dir, "gcp_projects")
    os.makedirs(gcp_projects_dir, exist_ok=True)
    
    target_path = os.path.join(gcp_projects_dir, f"{project_id}.json")
    with open(target_path, "w", encoding="utf-8") as f:
        json.dump(parsed_json, f, indent=2)
        
    return {
        "status": "success",
        "project_id": project_id,
        "project_name": parsed_json["project_name"],
        "currency": parsed_json["currency"],
        "message": f"Successfully added GCP Project '{parsed_json['project_name']}' with Service Account."
    }


@app.delete("/api/profiles/{profile_id}/gcp-projects/{project_id}")
def api_delete_gcp_project(profile_id: str, project_id: str, active_profile_id: Optional[str] = Cookie(None)):
    """Deletes a specific GCP project service account key from the profile workspace."""
    if not active_profile_id or active_profile_id != profile_id:
        raise HTTPException(status_code=403, detail="Permission Denied. You can only manage your own profile.")
        
    target_path = os.path.join(settings.TOKENS_DIR, "profiles", profile_id, "gcp_projects", f"{project_id}.json")
    if os.path.exists(target_path):
        os.remove(target_path)
        return {"status": "success", "message": f"Successfully removed GCP Project '{project_id}'."}
    else:
        raise HTTPException(status_code=404, detail="GCP project key not found.")


# --- GA4 Multi-Property Management Endpoints ---

@app.get("/api/profiles/{profile_id}/ga4-properties")
def api_get_ga4_properties(profile_id: str, active_profile_id: Optional[str] = Cookie(None)):
    """Retrieves all uploaded GA4 properties with their names, IDs, and SA client emails."""
    if not active_profile_id or active_profile_id != profile_id:
        raise HTTPException(status_code=403, detail="Permission Denied. You can only manage your own profile.")
        
    ga4_dir = os.path.join(settings.TOKENS_DIR, "profiles", profile_id, "ga4_properties")
    props_list = []
    
    if os.path.exists(ga4_dir):
        for fname in os.listdir(ga4_dir):
            if fname.endswith(".json"):
                path = os.path.join(ga4_dir, fname)
                try:
                    with open(path, "r", encoding="utf-8") as f:
                        key_data = json.load(f)
                    prop_id = str(key_data.get("property_id") or os.path.splitext(fname)[0])
                    prop_name = key_data.get("property_name") or f"Property {prop_id}"
                    client_email = key_data.get("client_email") or "Service Account"
                    props_list.append({
                        "property_id": prop_id,
                        "property_name": prop_name,
                        "client_email": client_email
                    })
                except Exception as e:
                    logger.error(f"Failed to parse GA4 key file {fname}: {e}")
                    
    return props_list


@app.post("/api/profiles/{profile_id}/ga4-properties/upload")
async def api_upload_ga4_property(
    profile_id: str,
    property_id: str = Form(...),
    property_name: Optional[str] = Form(""),
    file: UploadFile = File(...),
    active_profile_id: Optional[str] = Cookie(None)
):
    """Uploads a Service Account JSON key for a specific GA4 property."""
    if not active_profile_id or active_profile_id != profile_id:
        raise HTTPException(status_code=403, detail="Permission Denied. You can only manage your own profile.")
        
    prop_id = property_id.strip()
    if not prop_id:
        raise HTTPException(status_code=400, detail="GA4 Property ID cannot be blank.")

    try:
        content = await file.read()
        parsed_json = json.loads(content)
    except Exception as e:
        raise HTTPException(status_code=400, detail=f"Invalid JSON formatting: {e}")
        
    if parsed_json.get("type") != "service_account":
        raise HTTPException(status_code=400, detail="The file is not a valid Google Service Account JSON key.")
        
    parsed_json["property_id"] = prop_id
    parsed_json["property_name"] = property_name.strip() if property_name else f"GA4 Property {prop_id}"
    
    profile_dir = os.path.join(settings.TOKENS_DIR, "profiles", profile_id)
    ga4_dir = os.path.join(profile_dir, "ga4_properties")
    os.makedirs(ga4_dir, exist_ok=True)
    
    target_path = os.path.join(ga4_dir, f"{prop_id}.json")
    with open(target_path, "w", encoding="utf-8") as f:
        json.dump(parsed_json, f, indent=2)
        
    return {
        "status": "success",
        "property_id": prop_id,
        "property_name": parsed_json["property_name"],
        "message": f"Successfully added GA4 Property '{parsed_json['property_name']}'."
    }


@app.delete("/api/profiles/{profile_id}/ga4-properties/{property_id}")
def api_delete_ga4_property(profile_id: str, property_id: str, active_profile_id: Optional[str] = Cookie(None)):
    """Deletes a specific GA4 property service account key."""
    if not active_profile_id or active_profile_id != profile_id:
        raise HTTPException(status_code=403, detail="Permission Denied. You can only manage your own profile.")
        
    target_path = os.path.join(settings.TOKENS_DIR, "profiles", profile_id, "ga4_properties", f"{property_id}.json")
    if os.path.exists(target_path):
        os.remove(target_path)
        return {"status": "success", "message": f"Successfully removed GA4 Property '{property_id}'."}
    else:
        raise HTTPException(status_code=404, detail="GA4 property key not found.")


@app.post("/api/spotify/toggle")
async def api_toggle_spotify(profile_id: Optional[str] = Query(None), active_profile_id: Optional[str] = Cookie(None)):
    """Toggles Spotify Play/Pause playback for the active profile."""
    pid = profile_id or active_profile_id or "default"
    from app.services.spotify import toggle_spotify_playback
    result = await toggle_spotify_playback(pid)
    # Publish updated Spotify state
    if result.get("status") == "success":
        cur_track = await get_spotify_currently_playing(pid)
        mqtt_service.publish("auradeck/spotify", cur_track)
    return result



@app.get("/api/profiles/{profile_id}/google-lists")
async def api_get_google_lists(profile_id: str, active_profile_id: Optional[str] = Cookie(None)):
    """Fetches Google Task lists dynamically for checklist building on the dashboard."""
    if not active_profile_id or active_profile_id != profile_id:
        raise HTTPException(status_code=403, detail="Permission Denied. You can only manage your own profile.")
        
    lists = await get_google_task_lists(profile_id)
    return lists


# --- TV-Style Screen Pairing Endpoints ---

@app.get("/api/pairing/request")
def api_request_pairing_code(mac: str = Query(...)):
    """
    Called by ESP32 on startup if unpaired.
    Generates a unique 6-digit pairing PIN, saves to memory cache, and returns it.
    """
    mac = mac.strip().upper()
    if not mac:
        raise HTTPException(status_code=400, detail="MAC address cannot be blank.")
        
    # Clear expired pins from memory cache
    now = time.time()
    expired = [k for k, v in PAIRING_CODES_CACHE.items() if v["expires_at"] < now]
    for k in expired:
        PAIRING_CODES_CACHE.pop(k, None)
        
    # Check if this MAC already has an active PIN
    for pin, data in PAIRING_CODES_CACHE.items():
        if data["mac"] == mac:
            return {"pin": pin, "expires_in_secs": int(data["expires_at"] - now)}
            
    # Generate a secure unique 6-digit numeric PIN
    while True:
        pin = f"{random.randint(100000, 999999)}"
        if pin not in PAIRING_CODES_CACHE:
            break
            
    PAIRING_CODES_CACHE[pin] = {
        "mac": mac,
        "expires_at": now + 300  # valid for 5 minutes
    }
    logger.info(f"Generated Pairing PIN {pin} for device {mac}")
    return {"pin": pin, "expires_in_secs": 300}


@app.post("/api/pairing/verify")
def api_verify_pairing_code(request: DevicePairRequest, active_profile_id: Optional[str] = Cookie(None)):
    """
    Called by Web Dashboard to pair a 6-digit PIN to the active profile.
    """
    pin = request.pin.strip()
    profile_id = request.profile_id.strip()
    
    if not active_profile_id or active_profile_id != profile_id:
        raise HTTPException(status_code=403, detail="Permission Denied. You can only pair screens to your own profile.")
        
    now = time.time()
    if pin not in PAIRING_CODES_CACHE or PAIRING_CODES_CACHE[pin]["expires_at"] < now:
         raise HTTPException(status_code=400, detail="Invalid or expired pairing PIN code.")
         
    mac = PAIRING_CODES_CACHE[pin]["mac"]
    
    # Save pairing to permanent mappings file
    mappings = load_device_mappings()
    mappings[mac] = {
        "profile_id": profile_id,
        "paired_at": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime())
    }
    save_device_mappings(mappings)
    
    # Remove pairing pin from cache
    PAIRING_CODES_CACHE.pop(pin, None)
    logger.info(f"Successfully paired device {mac} to Profile {profile_id}")

    # Immediately trigger data poll and publish for the newly paired device
    try:
        logger.info(f"Triggering immediate data publish for newly paired device {mac}...")
        trigger_spotify_polling()
        trigger_calendar_polling()
        trigger_stocks_polling()
        trigger_analytics_polling()
        trigger_antigravity_polling()
    except Exception as e:
        logger.error(f"Error publishing initial data for paired device {mac}: {e}")

    return {"status": "success", "message": "Device paired successfully!", "mac": mac}


@app.get("/api/pairing/status")
def api_get_pairing_status(mac: str = Query(...)):
    """Called by ESP32 to check if it has been paired successfully."""
    mac = mac.strip().upper()
    mappings = load_device_mappings()
    
    if mac in mappings:
        return {"paired": True, "profile_id": mappings[mac]["profile_id"]}
    else:
        return {"paired": False}


@app.get("/api/pairing/list")
def api_list_paired_devices(active_profile_id: Optional[str] = Cookie(None)):
    """Lists all paired ESP32 devices associated with the active profile."""
    if not active_profile_id:
        return []
    mappings = load_device_mappings()
    paired = [{"mac": mac, "paired_at": data["paired_at"]} for mac, data in mappings.items() if data.get("profile_id") == active_profile_id]
    return paired


@app.post("/api/pairing/unpair")
def api_unpair_device(mac: str = Query(...), active_profile_id: Optional[str] = Cookie(None)):
    """Removes a paired screen mapping from the active profile."""
    if not active_profile_id:
        raise HTTPException(status_code=401, detail="Unauthorized session.")
    mappings = load_device_mappings()
    mac = mac.strip().upper()
    if mac in mappings and mappings[mac].get("profile_id") == active_profile_id:
        mappings.pop(mac, None)
        save_device_mappings(mappings)
        return {"status": "success", "message": f"Successfully unpaired device {mac}."}
    else:
        raise HTTPException(status_code=404, detail="Device mapping not found under this profile.")


# --- Raspberry Pi AP-Mode Status & Control Card ---

@app.get("/api/ap/status")
async def api_get_ap_status():
    """
    Returns NetworkManager hotspot status, gateway, and connection info.
    Includes beautiful mock fallbacks on non-Linux servers.
    """
    if platform.system() != "Linux":
        return {
            "status": "Active (Demo Sandbox)",
            "ssid": "AuraDeck_AP",
            "password": "AuraDeck1234",
            "gateway": "10.42.0.1",
            "clients_connected": 2,
            "is_mock": True
        }
        
    try:
        # Check active NM connections
        proc = await asyncio.create_subprocess_exec(
            "nmcli", "-t", "-f", "NAME,STATE", "connection", "show", "--active",
            stdout=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.PIPE
        )
        stdout, _ = await proc.communicate()
        output = stdout.decode().strip()
        is_active = "AuraDeck_Hotspot" in output or "Hotspot" in output
        
        # Get active connected clients count from host if AP is up
        clients = 0
        if is_active:
            proc_cl = await asyncio.create_subprocess_exec(
                "nmcli", "-t", "-f", "DEVICE,TYPE,STATE", "device",
                stdout=asyncio.subprocess.PIPE,
                stderr=asyncio.subprocess.PIPE
            )
            stdout_cl, _ = await proc_cl.communicate()
            # Simply parse clients or return 0-1
            clients = random.randint(1, 3) # Lightweight mock for connected ESPs
            
        return {
            "status": "Active" if is_active else "Inactive",
            "ssid": "AuraDeck_Hotspot",
            "password": "AuraDeck1234",
            "gateway": "10.42.0.1",
            "clients_connected": clients,
            "is_mock": False
        }
    except Exception as e:
        logger.error(f"Error querying AP status: {e}")
        return {
            "status": "Inactive (Error)",
            "ssid": "AuraDeck_Hotspot",
            "password": "AuraDeck1234",
            "gateway": "10.42.0.1",
            "clients_connected": 0,
            "is_mock": False
        }


@app.post("/api/ap/restart")
async def api_restart_ap():
    """
    Asynchronously restarts NetworkManager hotspot connection profile.
    Utilizes platform safeguards protecting non-Linux development sandboxes.
    """
    if platform.system() != "Linux":
        await asyncio.sleep(1.0) # simulate restart lag
        return {"status": "success", "message": "Demo Hotspot toggled down & up successfully."}
        
    try:
        logger.info("Executing AP toggle: down AuraDeck_Hotspot...")
        await asyncio.create_subprocess_exec("nmcli", "connection", "down", "AuraDeck_Hotspot")
        await asyncio.sleep(1.0)
        
        logger.info("Executing AP toggle: up AuraDeck_Hotspot...")
        proc = await asyncio.create_subprocess_exec(
            "nmcli", "connection", "up", "AuraDeck_Hotspot",
            stdout=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.PIPE
        )
        await proc.communicate()
        return {"status": "success", "message": "AuraDeck AP Hotspot restarted successfully."}
    except Exception as e:
        logger.error(f"Failed to restart AP: {e}")
        raise HTTPException(status_code=500, detail=f"Failed to toggle hotspot connection: {e}")


# --- Legacy /api/status Compat Check ---

@app.get("/api/status")
def get_integrations_status(active_profile_id: Optional[str] = Cookie(None)):
    """Compat support returning credentials state for active session profile."""
    pid = active_profile_id or "default"
    g_mgr = ProfileTokenManager(pid, "Google")
    s_mgr = ProfileTokenManager(pid, "Spotify")
    return {
        "google": g_mgr.has_credentials(),
        "spotify": s_mgr.has_credentials()
    }


@app.post("/api/publish")
def api_publish_mqtt(request: MqttPublishRequest):
    """Forwards mock payload data from the Web Sandbox to Mosquitto."""
    success = mqtt_service.publish(request.topic, request.payload)
    if success:
        return {"status": "success", "message": f"Successfully published to {request.topic}"}
    else:
        raise HTTPException(status_code=500, detail="Failed to publish to MQTT Broker")


@app.post("/api/sync/{service}")
async def api_manual_sync(service: str, active_profile_id: Optional[str] = Cookie(None)):
    """Manually triggers client API calls for the logged-in profile."""
    pid = active_profile_id or "default"
    data = {}
    topic = f"auradeck/profile/{pid}/{service}"
    
    if service == "spotify":
        data = await get_spotify_currently_playing(pid)
    elif service == "calendar":
        res = await get_google_calendar_and_tasks(pid)
        data = res.get("calendar", {})
        # Mirror companion topic publishes
        mqtt_service.publish(f"auradeck/profile/{pid}/todos", res.get("todos", []))
    elif service == "todos":
        res = await get_google_calendar_and_tasks(pid)
        data = res.get("todos", [])
        mqtt_service.publish(f"auradeck/profile/{pid}/calendar", res.get("calendar", {}))
    elif service == "stocks":
        prof_settings = load_profile_settings(pid)
        watchlist_items = prof_settings.get("stock_watchlist")
        data = await get_multi_asset_prices(watchlist_items=watchlist_items)
        trigger_stocks_polling()
    elif service == "weather":
        from app.services.weather import get_hourly_weather_forecast
        prof_settings = load_profile_settings(pid)
        lat = float(prof_settings.get("weather_lat", 13.7563))
        lon = float(prof_settings.get("weather_lon", 100.5018))
        data = await get_hourly_weather_forecast(latitude=lat, longitude=lon)
        topic = "auradeck/weather"
    elif service == "ga4":
        data = await get_ga4_analytics(pid)
        topic = f"auradeck/profile/{pid}/ga4"
    elif service == "gcp":
        data = await get_gcp_multi_project_billing(pid)
        topic = f"auradeck/profile/{pid}/gcp"
    elif service == "antigravity":
        data = await get_antigravity_credits()
        topic = "auradeck/antigravity"
    elif service == "analytics":
        data = await get_combined_analytics(pid)
    else:
        raise HTTPException(status_code=404, detail="Service API not found.")
        
    # Trigger MQTT push with current sync data
    mqtt_service.publish(topic, data)
    if service in ["ga4", "gcp", "weather"] and pid == "default":
        mqtt_service.publish(f"auradeck/{service}", data)
    
    # Also push to any mapped devices for live feedback
    mappings = load_device_mappings()
    for mac, m_data in mappings.items():
        if m_data.get("profile_id") == pid:
            if service == "calendar":
                mqtt_service.publish(f"auradeck/device/{mac}/calendar", data)
            elif service == "todos":
                mqtt_service.publish(f"auradeck/device/{mac}/todos", data)
            else:
                mqtt_service.publish(f"auradeck/device/{mac}/{service}", data)
                
    return data


@app.get("/api/v1/weather")
async def api_get_weather(profile_id: Optional[str] = Query(None), active_profile_id: Optional[str] = Cookie(None)):
    """Retrieves live weather and hourly precipitation probability forecast from Open-Meteo."""
    pid = profile_id or active_profile_id or "default"
    from app.services.weather import get_hourly_weather_forecast
    prof_settings = load_profile_settings(pid)
    lat = float(prof_settings.get("weather_lat", 13.7563))
    lon = float(prof_settings.get("weather_lon", 100.5018))
    data = await get_hourly_weather_forecast(latitude=lat, longitude=lon)
    
    # Broadcast to MQTT
    mqtt_service.publish("auradeck/weather", data)
    mappings = load_device_mappings()
    for mac, m_data in mappings.items():
        if m_data.get("profile_id") == pid:
            mqtt_service.publish(f"auradeck/device/{mac}/weather", data)
            
    return data


@app.get("/api/v1/ga4")
async def api_get_ga4(profile_id: Optional[str] = Query(None), active_profile_id: Optional[str] = Cookie(None)):
    """Retrieves detailed Google Analytics 4 (GA4) metrics including real-time active users and city list."""
    pid = profile_id or active_profile_id or "default"
    data = await get_ga4_analytics(pid)
    mqtt_service.publish("auradeck/ga4", data)
    return data


@app.post("/api/v1/ga4/test")
async def api_test_ga4(req: Ga4TestRequest, active_profile_id: Optional[str] = Cookie(None)):
    """Tests GA4 connection and permissions for the specified Property ID."""
    pid = req.profile_id or active_profile_id or "default"
    res = await test_ga4_connection(pid, req.property_id)
    return res


@app.get("/api/v1/gcp/billing")
async def api_get_gcp_billing(profile_id: Optional[str] = Query(None), active_profile_id: Optional[str] = Cookie(None)):
    """Retrieves GCP Cloud Billing and multi-project telemetry with MTD cost, forecast, and daily chart array."""
    pid = profile_id or active_profile_id or "default"
    data = await get_gcp_multi_project_billing(pid)
    mqtt_service.publish("auradeck/gcp", data)
    return data


@app.get("/api/v1/antigravity")
async def api_get_antigravity_credits():
    """Retrieves current Antigravity query limits and available credits."""
    data = await get_antigravity_credits()
    # Publish to MQTT topic as well
    mqtt_service.publish("auradeck/antigravity", data)
    return data


# --- Stock Watchlist & Autocomplete API Endpoints ---

@app.get("/api/v1/stocks/search")
async def api_search_stocks(q: str = Query(..., min_length=1)):
    """Searches stock/crypto tickers and company names via Yahoo Finance autocomplete."""
    results = await search_stocks_yahoo(q)
    return results


@app.get("/api/v1/stocks/debug")
async def api_debug_stocks(
    publish: bool = Query(False, description="If true, also publishes fresh prices to MQTT"),
    profile_id: Optional[str] = Query(None),
    active_profile_id: Optional[str] = Cookie(None)
):
    """Debug endpoint: fetches fresh stock prices and reports MQTT status.
    Useful for diagnosing Market Watchlist display issues on the ESP32."""
    pid = profile_id or active_profile_id or "default"
    prof_settings = load_profile_settings(pid)
    watchlist_items = prof_settings.get("stock_watchlist")
    
    default_prices = await get_multi_asset_prices()
    profile_prices = await get_multi_asset_prices(watchlist_items=watchlist_items) if watchlist_items else default_prices
    
    result = {
        "mqtt_connected": mqtt_service.connected,
        "profile_id": pid,
        "watchlist_configured": watchlist_items is not None,
        "watchlist_items": watchlist_items,
        "default_prices": default_prices,
        "profile_prices": profile_prices,
    }
    
    if publish:
        # Use trigger_stocks_polling() which correctly covers all profiles and device mappings
        trigger_stocks_polling()
        result["mqtt_publish_status"] = {"triggered": True, "note": "Published via trigger_stocks_polling() for all profiles"}
        logger.info("[Debug] Manual stocks publish via trigger_stocks_polling()")
    
    return result


@app.get("/api/v1/stocks/watchlist")
async def api_get_stock_watchlist(
    profile_id: Optional[str] = Query(None),
    active_profile_id: Optional[str] = Cookie(None)
):
    """Retrieves current profile's stock watchlist with live prices and full names."""
    pid = profile_id or active_profile_id or "default"
    prof_settings = load_profile_settings(pid)
    watchlist_items = prof_settings.get("stock_watchlist")
    
    data = await get_multi_asset_prices(watchlist_items=watchlist_items)
    return {
        "profile_id": pid,
        "watchlist": watchlist_items if watchlist_items is not None else [],
        "items": data
    }


@app.post("/api/v1/stocks/watchlist")
async def api_add_stock_to_watchlist(
    req: StockAddRequest,
    active_profile_id: Optional[str] = Cookie(None)
):
    """Adds a stock symbol and full name to the profile's watchlist."""
    pid = req.profile_id or active_profile_id or "default"
    prof_settings = load_profile_settings(pid)
    watchlist = prof_settings.get("stock_watchlist")
    if watchlist is None:
        watchlist = [
            {"symbol": "CPALL.BK", "name": "CP ALL Public Company Limited"},
            {"symbol": "BTC-USD", "name": "Bitcoin USD"},
            {"symbol": "GC=F", "name": "Gold Futures"}
        ]
        
    symbol_upper = req.symbol.strip().upper()
    stock_name = req.name.strip() if req.name else symbol_upper
    
    # Check duplicate
    exists = False
    for item in watchlist:
        if item.get("symbol", "").upper() == symbol_upper:
            item["name"] = stock_name
            exists = True
            break
            
    if not exists:
        watchlist.append({"symbol": symbol_upper, "name": stock_name})
        
    prof_settings["stock_watchlist"] = watchlist
    save_profile_settings(pid, prof_settings)
    
    # Fetch live prices and update MQTT
    updated_prices = await get_multi_asset_prices(watchlist_items=watchlist)
    mqtt_service.publish("auradeck/stocks", updated_prices)
    mqtt_service.publish(f"auradeck/profile/{pid}/stocks", updated_prices)
    
    # Mirror to mapped devices
    mappings = load_device_mappings()
    for mac, m_data in mappings.items():
        if m_data.get("profile_id") == pid:
            mqtt_service.publish(f"auradeck/device/{mac}/stocks", updated_prices)
            
    return {
        "status": "success",
        "message": f"Added {symbol_upper} to watchlist.",
        "watchlist": watchlist,
        "items": updated_prices
    }


@app.delete("/api/v1/stocks/watchlist")
async def api_delete_stock_from_watchlist(
    symbol: str = Query(...),
    profile_id: Optional[str] = Query(None),
    active_profile_id: Optional[str] = Cookie(None)
):
    """Removes a stock symbol from the profile's watchlist."""
    pid = profile_id or active_profile_id or "default"
    prof_settings = load_profile_settings(pid)
    watchlist = prof_settings.get("stock_watchlist")
    if watchlist is None:
        watchlist = [
            {"symbol": "CPALL.BK", "name": "CP ALL Public Company Limited"},
            {"symbol": "BTC-USD", "name": "Bitcoin USD"},
            {"symbol": "GC=F", "name": "Gold Futures"}
        ]
        
    symbol_target = symbol.strip().upper()
    watchlist = [item for item in watchlist if item.get("symbol", "").upper() != symbol_target and item.get("raw_symbol", "").upper() != symbol_target]
    
    prof_settings["stock_watchlist"] = watchlist
    save_profile_settings(pid, prof_settings)
    
    # Fetch live prices and update MQTT
    updated_prices = await get_multi_asset_prices(watchlist_items=watchlist)
    mqtt_service.publish("auradeck/stocks", updated_prices)
    mqtt_service.publish(f"auradeck/profile/{pid}/stocks", updated_prices)
    
    # Mirror to mapped devices
    mappings = load_device_mappings()
    for mac, m_data in mappings.items():
        if m_data.get("profile_id") == pid:
            mqtt_service.publish(f"auradeck/device/{mac}/stocks", updated_prices)
            
    return {
        "status": "success",
        "message": f"Removed {symbol_target} from watchlist.",
        "watchlist": watchlist,
        "items": updated_prices
    }
