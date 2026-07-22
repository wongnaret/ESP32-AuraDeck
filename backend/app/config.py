import os
from pydantic_settings import BaseSettings, SettingsConfigDict
from typing import Optional

class Settings(BaseSettings):
    MQTT_HOST: str = "mosquitto"
    MQTT_PORT: int = 1883
    
    GOOGLE_CLIENT_ID: str = "mock_google_client_id"
    GOOGLE_CLIENT_SECRET: str = "mock_google_client_secret"
    GOOGLE_REDIRECT_URI: str = "http://localhost:8000/google/callback"
    
    SPOTIFY_CLIENT_ID: str = "mock_spotify_client_id"
    SPOTIFY_CLIENT_SECRET: str = "mock_spotify_client_secret"
    SPOTIFY_REDIRECT_URI: str = "http://127.0.0.1:8000/spotify/callback"
    
    STOCK_API_KEY: str = "mock_stock_api_key"
    STOCK_WATCHLIST: str = "CPALL.BK,BTC-USD,GC=F"
    
    GCP_SERVICE_ACCOUNT_PATH: Optional[str] = None
    GCP_BILLING_PROJECT_IDS: str = "auradeck-dev,client-prod"
    
    GA4_PROPERTY_ID: str = "mock_property_id"
    GSC_SITE_URL: str = "https://mocksite.com"
    
    # Cache and local file storage paths
    TOKENS_DIR: str = "./tokens"
    
    model_config = SettingsConfigDict(
        env_file=os.path.join(os.path.dirname(os.path.dirname(__file__)), ".env"),
        env_file_encoding="utf-8",
        extra="ignore"
    )

# Ensure tokens directory exists
settings = Settings()
os.makedirs(settings.TOKENS_DIR, exist_ok=True)
