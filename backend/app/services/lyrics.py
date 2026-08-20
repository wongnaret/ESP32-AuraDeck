import re
import json
import logging
from typing import Dict, Any, List, Optional
import urllib.parse
import urllib.request

logger = logging.getLogger("lyrics_service")

# In-memory cache for parsed lyrics per track ID or title+artist
# Key: track_id / title_artist -> List[Dict[str, Any]] where each item is {"time_ms": int, "text": str}
LYRICS_CACHE: Dict[str, Dict[str, Any]] = {}

# Regex for LRC timestamp: [mm:ss.xx] or [mm:ss.xxx]
LRC_REGEX = re.compile(r"^\[(\d+):(\d+(?:\.\d+)?)\](.*)$")


def parse_lrc_lines(synced_lyrics: str) -> List[Dict[str, Any]]:
    """
    Parses an LRC format string into a sorted list of timestamped lines:
    [{"time_ms": 12340, "text": "Lyric text..."}, ...]
    """
    parsed: List[Dict[str, Any]] = []
    if not synced_lyrics:
        return parsed

    for line in synced_lyrics.splitlines():
        line = line.strip()
        match = LRC_REGEX.match(line)
        if match:
            try:
                minutes = int(match.group(1))
                seconds = float(match.group(2))
                text = match.group(3).strip()
                time_ms = int((minutes * 60 + seconds) * 1000)
                parsed.append({"time_ms": time_ms, "text": text})
            except Exception:
                continue

    # Ensure sorted by time
    parsed.sort(key=lambda x: x["time_ms"])
    return parsed


async def fetch_synced_lyrics(
    track_name: str,
    artist_name: str,
    album_name: Optional[str] = None,
    duration_secs: Optional[int] = None
) -> Optional[Dict[str, Any]]:
    """
    Calls LRCLIB public API (https://lrclib.net/) to fetch synced lyrics for a track.
    Returns parsed lyrics dictionary or None on failure/not found.
    """
    if not track_name or track_name in ("Unknown Track", "Not Playing", "Spotify Idle"):
        return None

    # Clean up track title (remove Spotify "(feat. ...)" or "- Remastered..." to increase LRCLIB hit rate)
    clean_title = re.sub(r"\(feat\..*?\)|\[feat\..*?\]|- Remaster.*?|\(Remaster.*?\)", "", track_name, flags=re.IGNORECASE).strip()
    primary_artist = artist_name.split(",")[0].strip() if artist_name else ""

    params = {
        "track_name": clean_title or track_name,
        "artist_name": primary_artist or artist_name
    }
    if album_name:
        params["album_name"] = album_name
    if duration_secs and duration_secs > 0:
        params["duration"] = str(duration_secs)

    query_string = urllib.parse.urlencode(params)
    full_url = f"https://lrclib.net/api/get?{query_string}"

    try:
        try:
            import httpx
            async with httpx.AsyncClient(headers={"User-Agent": "AuraDeck/2.0 (https://github.com/wongnaret/ESP32-AuraDeck)"}) as client:
                res = await client.get(full_url, timeout=4.0)
                status_code = res.status_code
                data = res.json() if status_code == 200 else {}
        except ImportError:
            import asyncio
            def sync_fetch():
                req = urllib.request.Request(full_url, headers={"User-Agent": "AuraDeck/2.0 (https://github.com/wongnaret/ESP32-AuraDeck)"})
                try:
                    with urllib.request.urlopen(req, timeout=4.0) as response:
                        return response.status, json.loads(response.read().decode('utf-8'))
                except urllib.error.HTTPError as e:
                    return e.code, {}
                except Exception:
                    return 500, {}
            loop = asyncio.get_event_loop()
            status_code, data = await loop.run_in_executor(None, sync_fetch)

        if status_code == 200:
            synced = data.get("syncedLyrics")
            is_instrumental = data.get("instrumental", False)
            
            if is_instrumental:
                return {
                    "instrumental": True,
                    "lines": []
                }

            if synced:
                parsed_lines = parse_lrc_lines(synced)
                return {
                    "instrumental": False,
                    "lines": parsed_lines
                }
        elif status_code == 404:
            logger.debug(f"LRCLIB: No synced lyrics found for '{track_name}' by '{artist_name}'")
        else:
            logger.debug(f"LRCLIB returned status {status_code} for '{track_name}'")
    except Exception as e:
        logger.debug(f"LRCLIB request error for '{track_name}': {e}")

    return None


async def get_active_lyrics_lines(
    track_id: str,
    track_name: str,
    artist_name: str,
    album_name: str,
    duration_ms: int,
    progress_ms: int
) -> Dict[str, Any]:
    """
    Returns the current and next lyrics line matching the track's current playback progress.
    Caches parsed lyrics in memory to prevent repeated API requests for the same track.
    Falls back to Album & Artist metadata when lyrics are unavailable.
    """
    cache_key = track_id or f"{track_name}_{artist_name}"
    
    lyrics_data = LYRICS_CACHE.get(cache_key)
    if lyrics_data is None:
        duration_secs = int(duration_ms / 1000) if duration_ms > 0 else None
        lyrics_data = await fetch_synced_lyrics(track_name, artist_name, album_name, duration_secs)
        # Store in cache (even if empty to avoid hammering API for unfound tracks)
        if lyrics_data is None:
            LYRICS_CACHE[cache_key] = {"instrumental": False, "lines": []}
        else:
            LYRICS_CACHE[cache_key] = lyrics_data

    cached_entry = LYRICS_CACHE[cache_key]
    lines = cached_entry.get("lines", [])
    is_instrumental = cached_entry.get("instrumental", False)

    # 1. Instrumental Track Fallback
    if is_instrumental:
        return {
            "has_lyrics": False,
            "current_lyric": "♪ Instrumental Track ♪",
            "next_lyric": f"Album: {album_name}" if album_name else ""
        }

    # 2. No Lyrics Available Fallback (Show Album & Artists per user preference)
    if not lines:
        return {
            "has_lyrics": False,
            "current_lyric": f"Album: {album_name}" if album_name else "♪ Enjoy the music ♪",
            "next_lyric": f"Artist: {artist_name}" if artist_name else ""
        }

    # 3. Time-matching for Synced Lyrics
    current_idx = -1
    for i, item in enumerate(lines):
        if item["time_ms"] <= progress_ms:
            current_idx = i
        else:
            break

    if current_idx == -1:
        # Song hasn't reached first lyric line yet (intro music)
        current_text = "♪ ♪ ♪"
        next_text = lines[0]["text"] if len(lines) > 0 else ""
    else:
        current_text = lines[current_idx]["text"]
        # Skip empty lines if present
        if not current_text and current_idx > 0:
            current_text = "♪ ♪ ♪"
            
        if current_idx + 1 < len(lines):
            next_text = lines[current_idx + 1]["text"]
        else:
            next_text = ""

    return {
        "has_lyrics": True,
        "current_lyric": current_text,
        "next_lyric": next_text
    }
