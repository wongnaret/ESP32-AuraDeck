import logging
import httpx
from typing import Dict, Any

from app.services.google_auth import spotify_tokens, refresh_spotify_token

logger = logging.getLogger("spotify_service")

async def get_spotify_currently_playing() -> Dict[str, Any]:
    """
    Fetches the currently playing track from Spotify.
    Handles token refreshing on 401 and returns a robust fallback state on any failure.
    """
    fallback_state = {
        "is_playing": False,
        "title": "Not Playing",
        "artist": "N/A",
        "progress_ms": 0,
        "duration_ms": 0
    }

    tokens = spotify_tokens.load_tokens()
    if not tokens or "access_token" not in tokens:
        logger.debug("Spotify credentials are not configured or missing.")
        return fallback_state

    access_token = tokens["access_token"]
    url = "https://api.spotify.com/v1/me/player/currently-playing"
    headers = {"Authorization": f"Bearer {access_token}"}

    async def fetch(token: str) -> httpx.Response:
        async with httpx.AsyncClient() as client:
            return await client.get(url, headers={"Authorization": f"Bearer {token}"}, timeout=5.0)

    try:
        response = await fetch(access_token)

        # Handle expired token gracefully
        if response.status_code == 401:
            logger.info("Spotify access token expired. Attempting token refresh...")
            new_access_token = await refresh_spotify_token()
            if new_access_token:
                response = await fetch(new_access_token)
            else:
                logger.error("Failed to refresh Spotify token.")
                return fallback_state

        if response.status_code == 204:
            # 204 means active session but nothing currently playing
            return fallback_state

        if response.status_code == 200:
            data = response.json()
            if not data or "item" not in data or data["item"] is None:
                return fallback_state

            item = data.get("item", {})
            artists = item.get("artists", [])
            artist_names = ", ".join([artist.get("name", "") for artist in artists]) if artists else "Unknown"
            
            return {
                "is_playing": data.get("is_playing", False),
                "title": item.get("name", "Unknown Title"),
                "artist": artist_names,
                "progress_ms": data.get("progress_ms", 0),
                "duration_ms": item.get("duration_ms", 0)
            }

        logger.warning(f"Spotify API returned unexpected status code: {response.status_code}")
        return fallback_state

    except httpx.RequestError as e:
        logger.error(f"Network error while calling Spotify API: {e}")
        return fallback_state
    except Exception as e:
        logger.error(f"Unexpected error in Spotify currently playing parser: {e}")
        return fallback_state
