import logging
import os
import asyncio
from fastapi import FastAPI, Request, HTTPException, Depends
from fastapi.responses import HTMLResponse, RedirectResponse
from fastapi.templating import Jinja2Templates
from pydantic import BaseModel
from apscheduler.schedulers.background import BackgroundScheduler

from app.config import settings
from app.services.mqtt_pub import mqtt_service
from app.services.google_auth import (
    google_tokens,
    spotify_tokens,
    get_google_auth_url,
    get_spotify_auth_url,
    exchange_google_code,
    exchange_spotify_code
)
from app.services.spotify import get_spotify_currently_playing
from app.services.google_api import get_google_calendar_and_tasks
from app.services.stocks import get_multi_asset_prices
from app.services.analytics import get_combined_analytics
from app.services.antigravity import get_antigravity_credits


# Setup logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger("backend_main")

app = FastAPI(
    title="AuraDeck Smart Dashboard Backend",
    description="FastAPI gateway managing API scheduling and MQTT push channels.",
    version="1.0.0"
)

# Setup HTML template rendering
templates_dir = os.path.join(os.path.dirname(__file__), "templates")
templates = Jinja2Templates(directory=templates_dir)

# APScheduler for polling background jobs
scheduler = BackgroundScheduler()


# --- Models for API Request Bodies ---

class MqttPublishRequest(BaseModel):
    topic: str
    payload: dict


# --- Startup & Shutdown Events ---

@app.on_event("startup")
def on_startup():
    logger.info("Starting up AuraDeck Control Center...")
    # 1. Establish connection to local Mosquitto broker
    mqtt_service.connect()
    
    # 2. Setup background polling scheduler
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
        minutes=15,
        max_instances=1
    )
    scheduler.add_job(
        id="stocks_polling_job",
        func=trigger_stocks_polling,
        trigger="interval",
        minutes=5,
        max_instances=1
    )
    scheduler.add_job(
        id="analytics_polling_job",
        func=trigger_analytics_polling,
        trigger="interval",
        minutes=15, # Poll analytics every 15 minutes to keep it responsive
        max_instances=1
    )
    scheduler.add_job(
        id="antigravity_polling_job",
        func=trigger_antigravity_polling,
        trigger="interval",
        minutes=30,
        max_instances=1
    )
    
    scheduler.start()
    logger.info("Background Schedulers started successfully.")


@app.on_event("shutdown")
def on_shutdown():
    logger.info("Shutting down background services...")
    mqtt_service.disconnect()
    scheduler.shutdown()


# --- Background Polling Triggers ---

def trigger_spotify_polling():
    """Polls Spotify currently-playing endpoint and publishes to MQTT."""
    try:
        data = asyncio.run(get_spotify_currently_playing())
        mqtt_service.publish("auradeck/spotify", data)
    except Exception as e:
        logger.error(f"Error in background Spotify poller: {e}")

def trigger_calendar_polling():
    """Polls Google Calendar & Tasks endpoints and publishes to MQTT."""
    try:
        data = asyncio.run(get_google_calendar_and_tasks())
        # Split calendar schedule and todo checklist into separate target topics
        mqtt_service.publish("auradeck/calendar", data.get("calendar", {}))
        mqtt_service.publish("auradeck/todos", data.get("todos", []))
    except Exception as e:
        logger.error(f"Error in background Calendar/Tasks poller: {e}")

def trigger_stocks_polling():
    """Polls stock prices, gold, and crypto, and publishes to MQTT."""
    try:
        data = asyncio.run(get_multi_asset_prices())
        mqtt_service.publish("auradeck/stocks", data)
    except Exception as e:
        logger.error(f"Error in background Stocks poller: {e}")

def trigger_analytics_polling():
    """Polls GA4 & GCP Billing costs and publishes to MQTT."""
    try:
        data = asyncio.run(get_combined_analytics())
        mqtt_service.publish("auradeck/analytics", data)
    except Exception as e:
        logger.error(f"Error in background Analytics poller: {e}")

def trigger_antigravity_polling():
    """Polls Google Antigravity credits and publishes to MQTT."""
    try:
        data = asyncio.run(get_antigravity_credits())
        mqtt_service.publish("auradeck/antigravity", data)
    except Exception as e:
        logger.error(f"Error in background Antigravity poller: {e}")


# --- Page Routing ---

@app.get("/", response_class=HTMLResponse)
@app.get("/auth", response_class=HTMLResponse)
def get_control_panel(request: Request):
    """Renders the main AuraDeck integration dashboard and developer sandbox."""
    return templates.TemplateResponse("auth.html", {"request": request})


# --- OAuth2 Redirection Endpoints ---

@app.get("/google/login")
def login_google():
    """Redirects the user to Google's OAuth2 login screen."""
    return RedirectResponse(url=get_google_auth_url())


@app.get("/google/callback")
async def callback_google(code: str):
    """Processes the callback code from Google, saving refresh tokens."""
    tokens = await exchange_google_code(code)
    if tokens:
        logger.info("Successfully completed Google OAuth2 authentication flow.")
        # Redirect back to home dashboard with success query param
        return RedirectResponse(url="/auth?google=success")
    else:
        raise HTTPException(status_code=400, detail="Google authentication failed.")


@app.get("/spotify/login")
def login_spotify():
    """Redirects the user to Spotify's OAuth2 login screen."""
    return RedirectResponse(url=get_spotify_auth_url())


@app.get("/spotify/callback")
async def callback_spotify(code: str):
    """Processes the callback code from Spotify, saving refresh tokens."""
    tokens = await exchange_spotify_code(code)
    if tokens:
        logger.info("Successfully completed Spotify OAuth2 authentication flow.")
        return RedirectResponse(url="/auth?spotify=success")
    else:
        raise HTTPException(status_code=400, detail="Spotify authentication failed.")


# --- REST API Endpoints for Dashboard UI ---

@app.get("/api/status")
def get_integrations_status():
    """Returns connected state of third-party API configurations."""
    return {
        "google": google_tokens.has_credentials(),
        "spotify": spotify_tokens.has_credentials()
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
async def api_manual_sync(service: str):
    """Manually triggers client API calls, publishing live payloads and returning them to sandbox."""
    data = {}
    topic = f"auradeck/{service}"
    
    if service == "spotify":
        data = await get_spotify_currently_playing()
    elif service == "calendar":
        res = await get_google_calendar_and_tasks()
        data = res.get("calendar", {})
        # Also publish companion todos for user interface consistency!
        mqtt_service.publish("auradeck/todos", res.get("todos", []))
    elif service == "todos":
        res = await get_google_calendar_and_tasks()
        data = res.get("todos", [])
        # Also publish companion calendar for user interface consistency!
        mqtt_service.publish("auradeck/calendar", res.get("calendar", {}))
    elif service == "stocks":
        data = await get_multi_asset_prices()
    elif service == "antigravity":
        data = await get_antigravity_credits()
    elif service == "analytics":
        data = await get_combined_analytics()
    else:
        raise HTTPException(status_code=404, detail="Service API not found.")
        
    # Trigger MQTT push with current sync data
    mqtt_service.publish(topic, data)
    return data
