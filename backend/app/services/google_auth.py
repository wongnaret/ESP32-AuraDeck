import json
import os
import logging
from typing import Optional, Dict, Any
import httpx
from app.config import settings

logger = logging.getLogger("auth_service")

def load_profile_settings(profile_id: str) -> Dict[str, Any]:
    """Loads settings.json for a specific profile."""
    profile_dir = os.path.join(settings.TOKENS_DIR, "profiles", profile_id)
    settings_path = os.path.join(profile_dir, "settings.json")
    if not os.path.exists(settings_path):
        return {}
    try:
        with open(settings_path, "r", encoding="utf-8") as f:
            return json.load(f)
    except Exception as e:
        logger.error(f"Failed to load settings for profile {profile_id}: {e}")
        return {}

def save_profile_settings(profile_id: str, data: Dict[str, Any]):
    """Saves settings.json for a specific profile."""
    profile_dir = os.path.join(settings.TOKENS_DIR, "profiles", profile_id)
    os.makedirs(profile_dir, exist_ok=True)
    settings_path = os.path.join(profile_dir, "settings.json")
    try:
        with open(settings_path, "w", encoding="utf-8") as f:
            json.dump(data, f, indent=2, ensure_ascii=False)
    except Exception as e:
        logger.error(f"Failed to save settings for profile {profile_id}: {e}")

class ProfileTokenManager:
    """Manages secure reading, writing, and auto-refreshing of OAuth2 tokens per profile."""
    
    def __init__(self, profile_id: str, service_name: str):
        self.profile_id = profile_id
        self.service_name = service_name
        self.profile_dir = os.path.join(settings.TOKENS_DIR, "profiles", profile_id)
        self.token_path = os.path.join(self.profile_dir, f"{service_name.lower()}_tokens.json")
        os.makedirs(self.profile_dir, exist_ok=True)

    def save_tokens(self, token_data: Dict[str, Any]):
        """Persists token dictionary to disk."""
        try:
            with open(self.token_path, "w", encoding="utf-8") as f:
                json.dump(token_data, f, indent=2, ensure_ascii=False)
            logger.info(f"Successfully saved {self.service_name} credentials for profile {self.profile_id} to disk.")
        except Exception as e:
            logger.error(f"Failed to save {self.service_name} credentials for profile {self.profile_id}: {e}")

    def load_tokens(self) -> Optional[Dict[str, Any]]:
        """Loads cached tokens from disk."""
        if not os.path.exists(self.token_path):
            return None
        try:
            with open(self.token_path, "r", encoding="utf-8") as f:
                return json.load(f)
        except Exception as e:
            logger.error(f"Failed to load {self.service_name} credentials for profile {self.profile_id}: {e}")
            return None

    def has_credentials(self) -> bool:
        """Checks if a valid refresh token exists on disk."""
        tokens = self.load_tokens()
        return tokens is not None and "refresh_token" in tokens


def get_google_auth_url(profile_id: str) -> str:
    """Generates the Google OAuth2 authorization URL."""
    # If this is the main login session, use global client_id and request identity + integration scopes
    if profile_id == "login_session":
        client_id = settings.GOOGLE_CLIENT_ID
        redirect_uri = settings.GOOGLE_REDIRECT_URI
        scopes = [
            "openid",
            "https://www.googleapis.com/auth/userinfo.email",
            "https://www.googleapis.com/auth/userinfo.profile",
            "https://www.googleapis.com/auth/calendar.readonly",
            "https://www.googleapis.com/auth/tasks.readonly"
        ]
    else:
        # Load from profile settings if available, fallback to global
        prof_settings = load_profile_settings(profile_id)
        client_id = prof_settings.get("google_client_id") or settings.GOOGLE_CLIENT_ID
        redirect_uri = prof_settings.get("google_redirect_uri") or settings.GOOGLE_REDIRECT_URI
        scopes = [
            "https://www.googleapis.com/auth/calendar.readonly",
            "https://www.googleapis.com/auth/tasks.readonly"
        ]
        
    params = {
        "client_id": client_id,
        "redirect_uri": redirect_uri,
        "response_type": "code",
        "scope": " ".join(scopes),
        "access_type": "offline",
        "prompt": "consent",
        "state": profile_id
    }
    query_string = "&".join(f"{k}={v}" for k, v in params.items())
    return f"https://accounts.google.com/o/oauth2/v2/auth?{query_string}"


async def exchange_google_code_for_login(code: str) -> Optional[Dict[str, Any]]:
    """Exchanges code, gets user profile info, creates profile folder, and returns tokens + profile_id."""
    client_id = settings.GOOGLE_CLIENT_ID
    client_secret = settings.GOOGLE_CLIENT_SECRET
    redirect_uri = settings.GOOGLE_REDIRECT_URI
    
    url = "https://oauth2.googleapis.com/token"
    data = {
        "code": code,
        "client_id": client_id,
        "client_secret": client_secret,
        "redirect_uri": redirect_uri,
        "grant_type": "authorization_code"
    }
    
    async with httpx.AsyncClient() as client:
        try:
            response = await client.post(url, data=data)
            if response.status_code != 200:
                logger.error(f"Google code exchange for login failed: {response.status_code} - {response.text}")
                return None
                
            token_data = response.json()
            access_token = token_data.get("access_token")
            if not access_token:
                logger.error("No access token found in Google OAuth response.")
                return None
                
            # Call Google Userinfo API to get email
            info_response = await client.get(
                "https://www.googleapis.com/oauth2/v3/userinfo",
                headers={"Authorization": f"Bearer {access_token}"}
            )
            if info_response.status_code != 200:
                logger.error(f"Google Userinfo API failed: {info_response.status_code} - {info_response.text}")
                return None
                
            user_info = info_response.json()
            email = user_info.get("email")
            if not email:
                logger.error("No email address found in Google Userinfo response.")
                return None
                
            # Sanitize email to form profile_id
            profile_id = email.replace("@", "_").replace(".", "_")
            
            # Create profile folder and settings if not exists
            profile_dir = os.path.join(settings.TOKENS_DIR, "profiles", profile_id)
            if not os.path.exists(profile_dir):
                os.makedirs(profile_dir, exist_ok=True)
                
            settings_path = os.path.join(profile_dir, "settings.json")
            if not os.path.exists(settings_path):
                # Save initial settings
                prof_name = user_info.get("name") or email.split("@")[0].title()
                with open(settings_path, "w", encoding="utf-8") as f:
                    json.dump({"profile_name": prof_name}, f, indent=2, ensure_ascii=False)
            
            # Save Google tokens inside profile folder
            mgr = ProfileTokenManager(profile_id, "Google")
            mgr.save_tokens(token_data)
            
            return {
                "profile_id": profile_id,
                "email": email,
                "name": user_info.get("name"),
                "tokens": token_data
            }
        except Exception as e:
            logger.error(f"Error during Google code exchange for login: {e}")
            return None


async def exchange_google_code(profile_id: str, code: str) -> Optional[Dict[str, Any]]:
    """Exchanges Google auth code for access & refresh tokens."""
    prof_settings = load_profile_settings(profile_id)
    client_id = prof_settings.get("google_client_id") or settings.GOOGLE_CLIENT_ID
    client_secret = prof_settings.get("google_client_secret") or settings.GOOGLE_CLIENT_SECRET
    redirect_uri = prof_settings.get("google_redirect_uri") or settings.GOOGLE_REDIRECT_URI
    
    url = "https://oauth2.googleapis.com/token"
    data = {
        "code": code,
        "client_id": client_id,
        "client_secret": client_secret,
        "redirect_uri": redirect_uri,
        "grant_type": "authorization_code"
    }
    async with httpx.AsyncClient() as client:
        try:
            response = await client.post(url, data=data)
            if response.status_code == 200:
                token_data = response.json()
                mgr = ProfileTokenManager(profile_id, "Google")
                mgr.save_tokens(token_data)
                return token_data
            else:
                logger.error(f"Google code exchange failed for profile {profile_id}: {response.status_code} - {response.text}")
                return None
        except Exception as e:
            logger.error(f"Error during Google code exchange for profile {profile_id}: {e}")
            return None


async def refresh_google_token(profile_id: str) -> Optional[str]:
    """Refreshes the Google access token silently using cached refresh token."""
    mgr = ProfileTokenManager(profile_id, "Google")
    cached = mgr.load_tokens()
    if not cached or "refresh_token" not in cached:
        logger.warning(f"No Google refresh token available for profile {profile_id} to refresh access token.")
        return None
        
    prof_settings = load_profile_settings(profile_id)
    client_id = prof_settings.get("google_client_id") or settings.GOOGLE_CLIENT_ID
    client_secret = prof_settings.get("google_client_secret") or settings.GOOGLE_CLIENT_SECRET
    
    url = "https://oauth2.googleapis.com/token"
    data = {
        "client_id": client_id,
        "client_secret": client_secret,
        "refresh_token": cached["refresh_token"],
        "grant_type": "refresh_token"
    }
    async with httpx.AsyncClient() as client:
        try:
            response = await client.post(url, data=data)
            if response.status_code == 200:
                new_data = response.json()
                if "refresh_token" not in new_data:
                    new_data["refresh_token"] = cached["refresh_token"]
                mgr.save_tokens(new_data)
                return new_data["access_token"]
            else:
                logger.error(f"Google token refresh failed for profile {profile_id}: {response.status_code} - {response.text}")
                return None
        except Exception as e:
            logger.error(f"Error during Google token refresh for profile {profile_id}: {e}")
            return None


# --- Spotify Auth Services ---

def get_spotify_auth_url(profile_id: str) -> str:
    """Generates the Spotify OAuth2 authorization URL."""
    prof_settings = load_profile_settings(profile_id)
    client_id = prof_settings.get("spotify_client_id") or settings.SPOTIFY_CLIENT_ID
    redirect_uri = prof_settings.get("spotify_redirect_uri") or settings.SPOTIFY_REDIRECT_URI
    
    scopes = ["user-read-currently-playing", "user-read-playback-state"]
    params = {
        "client_id": client_id,
        "redirect_uri": redirect_uri,
        "response_type": "code",
        "scope": " ".join(scopes),
        "show_dialog": "true",
        "state": profile_id
    }
    query_string = "&".join(f"{k}={v}" for k, v in params.items())
    return f"https://accounts.spotify.com/authorize?{query_string}"


async def exchange_spotify_code(profile_id: str, code: str) -> Optional[Dict[str, Any]]:
    """Exchanges Spotify auth code for access & refresh tokens."""
    prof_settings = load_profile_settings(profile_id)
    client_id = prof_settings.get("spotify_client_id") or settings.SPOTIFY_CLIENT_ID
    client_secret = prof_settings.get("spotify_client_secret") or settings.SPOTIFY_CLIENT_SECRET
    redirect_uri = prof_settings.get("spotify_redirect_uri") or settings.SPOTIFY_REDIRECT_URI
    
    url = "https://accounts.spotify.com/api/token"
    data = {
        "code": code,
        "redirect_uri": redirect_uri,
        "grant_type": "authorization_code"
    }
    auth = (client_id, client_secret)
    async with httpx.AsyncClient() as client:
        try:
            response = await client.post(url, data=data, auth=auth)
            if response.status_code == 200:
                token_data = response.json()
                mgr = ProfileTokenManager(profile_id, "Spotify")
                mgr.save_tokens(token_data)
                return token_data
            else:
                logger.error(f"Spotify code exchange failed for profile {profile_id}: {response.status_code} - {response.text}")
                return None
        except Exception as e:
            logger.error(f"Error during Spotify code exchange for profile {profile_id}: {e}")
            return None


async def refresh_spotify_token(profile_id: str) -> Optional[str]:
    """Refreshes the Spotify access token silently."""
    mgr = ProfileTokenManager(profile_id, "Spotify")
    cached = mgr.load_tokens()
    if not cached or "refresh_token" not in cached:
        logger.warning(f"No Spotify refresh token available for profile {profile_id} to refresh access token.")
        return None
        
    prof_settings = load_profile_settings(profile_id)
    client_id = prof_settings.get("spotify_client_id") or settings.SPOTIFY_CLIENT_ID
    client_secret = prof_settings.get("spotify_client_secret") or settings.SPOTIFY_CLIENT_SECRET
    
    url = "https://accounts.spotify.com/api/token"
    data = {
        "grant_type": "refresh_token",
        "refresh_token": cached["refresh_token"]
    }
    auth = (client_id, client_secret)
    async with httpx.AsyncClient() as client:
        try:
            response = await client.post(url, data=data, auth=auth)
            if response.status_code == 200:
                new_data = response.json()
                if "refresh_token" not in new_data:
                    new_data["refresh_token"] = cached["refresh_token"]
                mgr.save_tokens(new_data)
                return new_data["access_token"]
            else:
                logger.error(f"Spotify token refresh failed for profile {profile_id}: {response.status_code} - {response.text}")
                return None
        except Exception as e:
            logger.error(f"Error during Spotify token refresh for profile {profile_id}: {e}")
            return None
