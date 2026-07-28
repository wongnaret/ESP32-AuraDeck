import logging
import httpx
from typing import Dict, Any

from app.services.google_auth import ProfileTokenManager, refresh_spotify_token
from app.config import settings

logger = logging.getLogger("spotify_service")

import io
import os
import hashlib
from PIL import Image

async def process_monochrome_album_art(image_url: str, profile_id: str) -> str:
    """
    Downloads Spotify cover image, resizes to 80x80 pixels, and converts to 1-bit Floyd-Steinberg dithered image.
    Saves PNG (for Web Dashboard preview) and BMP (for ESP32 hardware display).
    Returns unique image hash string.
    """
    if not image_url:
        return ""
        
    url_hash = hashlib.md5(image_url.encode('utf-8')).hexdigest()[:10]
    out_dir = os.path.join(settings.TOKENS_DIR, "profiles", profile_id)
    os.makedirs(out_dir, exist_ok=True)
    
    png_path = os.path.join(out_dir, "spotify_cover.png")
    bmp_path = os.path.join(out_dir, "spotify_cover.bmp")
    hash_path = os.path.join(out_dir, "spotify_cover.hash")
    
    if os.path.exists(hash_path):
        try:
            with open(hash_path, "r", encoding="utf-8") as f:
                if f.read().strip() == url_hash and os.path.exists(png_path) and os.path.exists(bmp_path):
                    return url_hash
        except Exception:
            pass

    try:
        async with httpx.AsyncClient() as client:
            res = await client.get(image_url, timeout=5.0)
            if res.status_code == 200:
                img = Image.open(io.BytesIO(res.content)).convert("RGB")
                img = img.resize((80, 80), Image.Resampling.LANCZOS)
                
                # Convert to 1-bit Floyd-Steinberg dithered monochrome
                mono_img = img.convert("1", dither=Image.Dither.FLOYDSTEINBERG)
                mono_img.save(png_path, format="PNG")
                mono_img.save(bmp_path, format="BMP")
                
                with open(hash_path, "w", encoding="utf-8") as f:
                    f.write(url_hash)
                    
                logger.info(f"Generated 1-bit monochrome Spotify cover art for profile {profile_id} (hash: {url_hash})")
                return url_hash
    except Exception as e:
        logger.error(f"Failed to generate monochrome cover art: {e}")
        
    return ""

async def get_spotify_currently_playing(profile_id: str = "default") -> Dict[str, Any]:
    """
    Fetches the currently playing track from Spotify for a specific profile.
    Handles token refreshing on 401 and returns a robust fallback state on any failure.
    """
    fallback_state = {
        "is_playing": False,
        "title": "Not Playing",
        "artist": "N/A",
        "progress_ms": 0,
        "duration_ms": 0,
        "album_art_url": "",
        "cover_hash": ""
    }

    mgr = ProfileTokenManager(profile_id, "Spotify")
    tokens = mgr.load_tokens()
    if not tokens or "access_token" not in tokens:
        logger.debug(f"Spotify credentials are not configured or missing for profile {profile_id}.")
        return fallback_state

    access_token = tokens["access_token"]
    url = "https://api.spotify.com/v1/me/player/currently-playing"

    async def fetch(token: str) -> httpx.Response:
        async with httpx.AsyncClient() as client:
            return await client.get(url, headers={"Authorization": f"Bearer {token}"}, timeout=5.0)

    try:
        response = await fetch(access_token)

        # Handle expired token gracefully
        if response.status_code == 401:
            logger.info(f"Spotify access token expired for profile {profile_id}. Attempting token refresh...")
            new_access_token = await refresh_spotify_token(profile_id)
            if new_access_token:
                response = await fetch(new_access_token)
            else:
                logger.error(f"Failed to refresh Spotify token for profile {profile_id}.")
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
            
            # Extract smallest / medium album cover image URL
            album_art_url = ""
            album = item.get("album", {})
            images = album.get("images", [])
            if images:
                album_art_url = images[0].get("url", "")

            cover_hash = ""
            if album_art_url:
                cover_hash = await process_monochrome_album_art(album_art_url, profile_id)

            return {
                "is_playing": data.get("is_playing", False),
                "title": item.get("name", "Unknown Title"),
                "artist": artist_names,
                "progress_ms": data.get("progress_ms", 0),
                "duration_ms": item.get("duration_ms", 0),
                "album_art_url": album_art_url,
                "cover_hash": cover_hash
            }

        logger.warning(f"Spotify API returned unexpected status code: {response.status_code} for profile {profile_id}")
        return fallback_state

    except httpx.RequestError as e:
        logger.error(f"Network error while calling Spotify API for profile {profile_id}: {e}")
        return fallback_state
    except Exception as e:
        logger.error(f"Unexpected error in Spotify currently playing parser for profile {profile_id}: {e}")
        return fallback_state
