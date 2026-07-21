import logging
import os
from fastapi import FastAPI, Request, HTTPException, Depends
from fastapi.responses import HTML_Response, RedirectResponse
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
    
    scheduler.start()
    logger.info("Background Schedulers started successfully.")


@app.on_event("shutdown")
def on_shutdown():
    logger.info("Shutting down background services...")
    mqtt_service.disconnect()
    scheduler.shutdown()


# --- Background Polling Triggers (Skeletons to be fully implemented in Phase 2) ---

def trigger_spotify_polling():
    """Polls Spotify currently-playing endpoint and publishes to MQTT."""
    # Logic will be implemented in Phase 2, currently acts as background runner
    pass

def trigger_calendar_polling():
    """Polls Google Calendar & Tasks endpoints and publishes to MQTT."""
    pass

def trigger_stocks_polling():
    """Polls AlphaVantage/Yahoo stock prices and publishes to MQTT."""
    pass


# --- Page Routing ---

@app.get("/", response_class=HTML_Response)
@app.get("/auth", response_class=HTML_Response)
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
    # In Phase 1, we will return Mock values. In Phase 2, we will integrate true client API fetch logic.
    mock_data = {}
    
    if service == "spotify":
        mock_data = {
            "is_playing": True,
            "title": "เพลงรักในสายลม (Live)",
            "artist": "วงดนตรีสากล",
            "progress_ms": 134000,
            "duration_ms": 240000
        }
    elif service == "calendar":
        mock_data = {
            "month_days_with_events": [1, 5, 12, 15, 20, 21, 22, 28],
            "events": [
                { "time": "14:00", "title": "ประชุมทีมสถาปัตยกรรม (Sync)", "is_today": true },
                { "time": "Tomorrow 10:00", "title": "สแตนด์อัปรายวัน", "is_today": false }
            ]
        }
    elif service == "todos":
        mock_data = [
            { "id": "1", "title": "ตรวจทาน Pull Request #42", "completed": False },
            { "id": "2", "title": "ติดตั้งโปรแกรมปรับปรุงระบบฐานข้อมูล", "completed": False }
        ]
    elif service == "stocks":
        mock_data = [
            { "symbol": "CPALL", "price": 57.25, "change_pct": 1.33, "type": "TH_STOCK" },
            { "symbol": "BTC/THB", "price": 2350000.00, "change_pct": 2.15, "type": "CRYPTO" },
            { "symbol": "GOLD_TH", "price": 41200.00, "change_pct": -0.24, "type": "GOLD" }
        ]
    elif service == "antigravity":
        mock_data = {
            "limit_5h": { "used": 12.5, "total": 50.0, "percentage": 25.0 },
            "limit_weekly": { "used": 140.0, "total": 500.0, "percentage": 28.0 },
            "next_reset": "02h 15m"
        }
    elif service == "analytics":
        mock_data = {
            "gcp_status": "OK",
            "ga4_active_users": 34,
            "gsc_clicks": 1420,
            "gsc_impressions": 28500,
            "gcp_billing": [
                { "project_name": "AuraDeck Dev", "cost_mtd": 12.50, "currency": "USD" },
                { "project_name": "Client Prod", "cost_mtd": 148.20, "currency": "USD" }
            ]
        }
    else:
        raise HTTPException(status_code=404, detail="Service API not found.")
        
    # Trigger MQTT push with current sync data
    mqtt_service.publish(f"auradeck/{service}", mock_data)
    return mock_data
