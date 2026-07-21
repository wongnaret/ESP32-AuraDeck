import json
import os
import logging
from typing import Optional, Dict, Any
import httpx
from app.config import settings

logger = logging.getLogger("auth_service")

class TokenManager:
    """Manages secure reading, writing, and auto-refreshing of OAuth2 tokens."""
    
    def __init__(self, service_name: str, token_filename: str):
        self.service_name = service_name
        self.token_path = os.path.join(settings.TOKENS_DIR, token_filename)
        os.makedirs(settings.TOKENS_DIR, exist_ok=True)

    def save_tokens(self, token_data: Dict[str, Any]):
        """Persists token dictionary to disk."""
        try:
            with open(self.token_path, "w", encoding="utf-8") as f:
                json.dump(token_data, f, indent=2, ensure_ascii=False)
            logger.info(f"Successfully saved {self.service_name} credentials to disk.")
        except Exception as e:
            logger.error(f"Failed to save {self.service_name} credentials: {e}")

    def load_tokens(self) -> Optional[Dict[str, Any]]:
        """Loads cached tokens from disk."""
        if not os.path.exists(self.token_path):
            return None
        try:
            with open(self.token_path, "r", encoding="utf-8") as f:
                return json.load(f)
        except Exception as e:
            logger.error(f"Failed to load {self.service_name} credentials: {e}")
            return None

    def has_credentials(self) -> bool:
        """Checks if a valid refresh token exists on disk."""
        tokens = self.load_tokens()
        return tokens is not None and "refresh_token" in tokens


# Singletons for both Google and Spotify
google_tokens = TokenManager("Google", "google_tokens.json")
spotify_tokens = TokenManager("Spotify", "spotify_tokens.json")


def get_google_auth_url() -> str:
    """Generates the Google OAuth2 authorization URL."""
    scopes = [
        "https://www.googleapis.com/auth/calendar.readonly",
        "https://www.googleapis.com/auth/tasks.readonly"
    ]
    params = {
        "client_id": settings.GOOGLE_CLIENT_ID,
        "redirect_uri": settings.GOOGLE_REDIRECT_URI,
        "response_type": "code",
        "scope": " ".join(scopes),
        "access_type": "offline",
        "prompt": "consent"
    }
    query_string = "&".join(f"{k}={v}" for k, v in params.items())
    return f"https://accounts.google.com/o/oauth2/v2/auth?{query_string}"


async def exchange_google_code(code: str) -> Optional[Dict[str, Any]]:
    """Exchanges Google auth code for access & refresh tokens."""
    url = "https://oauth2.googleapis.com/token"
    data = {
        "code": code,
        "client_id": settings.GOOGLE_CLIENT_ID,
        "client_secret": settings.GOOGLE_CLIENT_SECRET,
        "redirect_uri": settings.GOOGLE_REDIRECT_URI,
        "grant_type": "authorization_code"
    }
    async with httpx.AsyncClient() as client:
        try:
            response = await client.post(url, data=data)
            if response.status_code == 200:
                token_data = response.json()
                google_tokens.save_tokens(token_data)
                return token_data
            else:
                logger.error(f"Google code exchange failed: {response.status_code} - {response.text}")
                return None
        except Exception as e:
            logger.error(f"Error during Google code exchange: {e}")
            return None


async def refresh_google_token() -> Optional[str]:
    """Refreshes the Google access token silently using cached refresh token."""
    cached = google_tokens.load_tokens()
    if not cached or "refresh_token" not in cached:
        logger.warning("No Google refresh token available to refresh access token.")
        return None
        
    url = "https://oauth2.googleapis.com/token"
    data = {
        "client_id": settings.GOOGLE_CLIENT_ID,
        "client_secret": settings.GOOGLE_CLIENT_SECRET,
        "refresh_token": cached["refresh_token"],
        "grant_type": "refresh_token"
    }
    async with httpx.AsyncClient() as client:
        try:
            response = await client.post(url, data=data)
            if response.status_code == 200:
                new_data = response.json()
                # Google might not send a new refresh token, preserve old one
                if "refresh_token" not in new_data:
                    new_data["refresh_token"] = cached["refresh_token"]
                google_tokens.save_tokens(new_data)
                return new_data["access_token"]
            else:
                logger.error(f"Google token refresh failed: {response.status_code} - {response.text}")
                return None
        except Exception as e:
            logger.error(f"Error during Google token refresh: {e}")
            return None


# --- Spotify Auth Services ---

def get_spotify_auth_url() -> str:
    """Generates the Spotify OAuth2 authorization URL."""
    scopes = ["user-read-currently-playing", "user-read-playback-state"]
    params = {
        "client_id": settings.SPOTIFY_CLIENT_ID,
        "redirect_uri": settings.SPOTIFY_REDIRECT_URI,
        "response_type": "code",
        "scope": " ".join(scopes),
        "show_dialog": "true"
    }
    query_string = "&".join(f"{k}={v}" for k, v in params.items())
    return f"https://accounts.spotify.com/authorize?{query_string}"


async def exchange_spotify_code(code: str) -> Optional[Dict[str, Any]]:
    """Exchanges Spotify auth code for access & refresh tokens."""
    url = "https://accounts.spotify.com/api/token"
    data = {
        "code": code,
        "redirect_uri": settings.SPOTIFY_REDIRECT_URI,
        "grant_type": "authorization_code"
    }
    # Spotify OAuth requires Client ID and Secret in HTTP Basic Auth, or POST body
    auth = (settings.SPOTIFY_CLIENT_ID, settings.SPOTIFY_CLIENT_SECRET)
    async with httpx.AsyncClient() as client:
        try:
            response = await client.post(url, data=data, auth=auth)
            if response.status_code == 200:
                token_data = response.json()
                spotify_tokens.save_tokens(token_data)
                return token_data
            else:
                logger.error(f"Spotify code exchange failed: {response.status_code} - {response.text}")
                return None
        except Exception as e:
            logger.error(f"Error during Spotify code exchange: {e}")
            return None


async def refresh_spotify_token() -> Optional[str]:
    """Refreshes the Spotify access token silently."""
    cached = spotify_tokens.load_tokens()
    if not cached or "refresh_token" not in cached:
        logger.warning("No Spotify refresh token available to refresh access token.")
        return None
        
    url = "https://accounts.spotify.com/api/token"
    data = {
        "grant_type": "refresh_token",
        "refresh_token": cached["refresh_token"]
    }
    auth = (settings.SPOTIFY_CLIENT_ID, settings.SPOTIFY_CLIENT_SECRET)
    async with httpx.AsyncClient() as client:
        try:
            response = await client.post(url, data=data, auth=auth)
            if response.status_code == 200:
                new_data = response.json()
                if "refresh_token" not in new_data:
                    new_data["refresh_token"] = cached["refresh_token"]
                spotify_tokens.save_tokens(new_data)
                return new_data["access_token"]
            else:
                logger.error(f"Spotify token refresh failed: {response.status_code} - {response.text}")
                return None
        except Exception as e:
            logger.error(f"Error during Spotify token refresh: {e}")
            return None
