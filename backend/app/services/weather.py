import re
import json
import logging
from datetime import datetime, timezone, timedelta
from typing import Dict, Any, List, Optional
import urllib.parse
import urllib.request

logger = logging.getLogger("weather_service")

THAI_DAYS = ["วันจันทร์", "วันอังคาร", "วันพุธ", "วันพฤหัสบดี", "วันศุกร์", "วันเสาร์", "วันอาทิตย์"]
THAI_MONTHS = [
    "มกราคม", "กุมภาพันธ์", "มีนาคม", "เมษายน", "พฤษภาคม", "มิถุนายน",
    "กรกฎาคม", "สิงหาคม", "กันยายน", "ตุลาคม", "พฤศจิกายน", "ธันวาคม"
]

EN_DAYS = ["Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"]
EN_MONTHS = [
    "January", "February", "March", "April", "May", "June",
    "July", "August", "September", "October", "November", "December"
]


def format_dual_date(now: Optional[datetime] = None) -> Dict[str, str]:
    """
    Formats the date in English and Thai (Buddhist Era year + 543).
    Example:
      EN: "Thursday, 20 August 2026"
      TH: "วันพฤหัสบดีที่ 20 สิงหาคม 2569"
    """
    if now is None:
        # Thailand timezone GMT+7
        tz_th = timezone(timedelta(hours=7))
        now = datetime.now(tz_th)

    day_idx = now.weekday()
    day_num = now.day
    month_idx = now.month - 1
    year_ce = now.year
    year_be = year_ce + 543

    date_en = f"{EN_DAYS[day_idx]}, {day_num} {EN_MONTHS[month_idx]} {year_ce}"
    date_th = f"{THAI_DAYS[day_idx]}ที่ {day_num} {THAI_MONTHS[month_idx]} {year_be}"

    return {
        "date_en": date_en,
        "date_th": date_th
    }


def map_wmo_weather_code(code: int) -> Dict[str, str]:
    """
    Maps WMO weather codes to condition names and clean symbols.
    """
    if code == 0:
        return {"name_th": "ท้องฟ้าแจ่มใส", "condition": "Clear", "icon": "SUN"}
    elif code in (1, 2):
        return {"name_th": "มีเมฆบางส่วน", "condition": "Partly Cloudy", "icon": "P_CLOUD"}
    elif code == 3:
        return {"name_th": "มีเมฆมาก", "condition": "Cloudy", "icon": "CLOUD"}
    elif code in (45, 48):
        return {"name_th": "มีหมอกหนา", "condition": "Fog", "icon": "FOG"}
    elif code in (51, 53, 55):
        return {"name_th": "ฝนตกปรอยๆ", "condition": "Drizzle", "icon": "DRIZZLE"}
    elif code in (61, 63, 65):
        return {"name_th": "ฝนตกปานกลาง", "condition": "Rain", "icon": "RAIN"}
    elif code in (80, 81, 82):
        return {"name_th": "ฝนตกเป็นช่วงๆ", "condition": "Showers", "icon": "RAIN"}
    elif code in (95, 96, 99):
        return {"name_th": "ฝนฟ้าคะนอง", "condition": "Thunderstorm", "icon": "STORM"}
    else:
        return {"name_th": "สภาพอากาศทั่วไป", "condition": "Fair", "icon": "SUN"}


# Cache for reverse geocoding to prevent repetitive network requests
_GEOCODE_CACHE: Dict[str, Dict[str, str]] = {}


async def resolve_location_lines(
    location_name: Optional[str] = None,
    latitude: float = 13.7563,
    longitude: float = 100.5018
) -> Dict[str, str]:
    """
    Resolves location into line1 (POI/suburb/district) and line2 (province/city).
    Supports custom user-defined location names or automatic reverse geocoding via Nominatim.
    """
    if location_name and location_name.strip():
        loc_str = location_name.strip()
        parts = [p.strip() for p in loc_str.split(",") if p.strip()]
        if len(parts) >= 3:
            return {
                "location_name": loc_str,
                "location_line1": parts[0],
                "location_line2": ", ".join(parts[1:])
            }
        elif len(parts) == 2:
            return {
                "location_name": loc_str,
                "location_line1": parts[0],
                "location_line2": parts[1]
            }
        else:
            return {
                "location_name": loc_str,
                "location_line1": loc_str,
                "location_line2": "สภาพอากาศ"
            }

    # Reverse Geocoding with coordinate-key caching
    cache_key = f"{latitude:.3f},{longitude:.3f}"
    if cache_key in _GEOCODE_CACHE:
        return _GEOCODE_CACHE[cache_key]

    default_result = {
        "location_name": "กรุงเทพมหานคร, ประเทศไทย",
        "location_line1": "กรุงเทพมหานคร",
        "location_line2": "ประเทศไทย"
    }

    try:
        url = f"https://nominatim.openstreetmap.org/reverse?format=json&lat={latitude:.4f}&lon={longitude:.4f}&zoom=14&addressdetails=1"
        req = urllib.request.Request(
            url,
            headers={
                "User-Agent": "ESP32-AuraDeck/2.0 (weather_service)",
                "Accept-Language": "th,en"
            }
        )
        
        import asyncio
        def fetch_geo():
            try:
                with urllib.request.urlopen(req, timeout=3.0) as resp:
                    if resp.status == 200:
                        return json.loads(resp.read().decode('utf-8'))
            except Exception as ex:
                logger.debug(f"Reverse geocode network fetch error: {ex}")
            return None

        loop = asyncio.get_event_loop()
        geo_data = await loop.run_in_executor(None, fetch_geo)

        if geo_data:
            addr = geo_data.get("address", {})
            poi = (
                geo_data.get("name")
                or addr.get("suburb")
                or addr.get("neighbourhood")
                or addr.get("amenity")
                or addr.get("building")
                or addr.get("village")
                or ""
            )
            district = (
                addr.get("district")
                or addr.get("subdistrict")
                or addr.get("county")
                or addr.get("city_district")
                or ""
            )
            city = (
                addr.get("city")
                or addr.get("town")
                or addr.get("province")
                or addr.get("state")
                or ""
            )

            if poi and (district or city):
                line1 = poi
                line2 = ", ".join(filter(None, [district, city]))
            elif district and city:
                line1 = district
                line2 = city
            elif city:
                line1 = city
                line2 = "ประเทศไทย"
            else:
                line1 = f"{latitude:.2f}°N, {longitude:.2f}°E"
                line2 = "สภาพอากาศ"

            full_name = ", ".join(filter(None, [poi, district, city])) or f"{latitude:.4f}, {longitude:.4f}"
            res = {
                "location_name": full_name,
                "location_line1": line1,
                "location_line2": line2
            }
            _GEOCODE_CACHE[cache_key] = res
            return res

    except Exception as e:
        logger.warning(f"Failed to reverse-geocode coordinates ({latitude}, {longitude}): {e}")

    return default_result


async def get_hourly_weather_forecast(
    latitude: float = 13.7563,
    longitude: float = 100.5018,
    timezone_str: str = "Asia/Bangkok",
    location_name: Optional[str] = None
) -> Dict[str, Any]:
    """
    Fetches live weather and next 6 hours precipitation probability forecast from Open-Meteo API.
    Returns structured JSON with location details, dual-language dates, and hourly rain forecast list.
    """
    tz_th = timezone(timedelta(hours=7))
    now = datetime.now(tz_th)
    date_info = format_dual_date(now)
    loc_info = await resolve_location_lines(location_name=location_name, latitude=latitude, longitude=longitude)

    fallback_payload = {
        "location_name": loc_info["location_name"],
        "location_line1": loc_info["location_line1"],
        "location_line2": loc_info["location_line2"],
        "date_en": date_info["date_en"],
        "date_th": date_info["date_th"],
        "current_temp": 29.0,
        "current_condition": "มีเมฆเป็นส่วนมาก",
        "current_icon": "CLOUD",
        "hourly": [
            {"time": f"{(now.hour + i) % 24:02d}:00", "condition": "Rain" if i < 3 else "Cloudy", "icon": "RAIN" if i < 3 else "CLOUD", "rain_prob": max(0, 70 - i * 15), "temp": 28.5}
            for i in range(1, 7)
        ]
    }

    params = {
        "latitude": f"{latitude:.4f}",
        "longitude": f"{longitude:.4f}",
        "current": "temperature_2m,weather_code",
        "hourly": "temperature_2m,precipitation_probability,weather_code",
        "timezone": timezone_str,
        "forecast_days": "2"
    }

    query_str = urllib.parse.urlencode(params)
    url = f"https://api.open-meteo.com/v1/forecast?{query_str}"

    try:
        try:
            import httpx
            async with httpx.AsyncClient() as client:
                res = await client.get(url, timeout=5.0)
                status_code = res.status_code
                data = res.json() if status_code == 200 else {}
        except ImportError:
            import asyncio
            def sync_fetch():
                req = urllib.request.Request(url, headers={"User-Agent": "AuraDeck/2.0"})
                try:
                    with urllib.request.urlopen(req, timeout=5.0) as response:
                        return response.status, json.loads(response.read().decode('utf-8'))
                except Exception:
                    return 500, {}
            loop = asyncio.get_event_loop()
            status_code, data = await loop.run_in_executor(None, sync_fetch)

        if status_code != 200 or not data:
            logger.warning(f"Open-Meteo API returned status {status_code}, using fallback.")
            return fallback_payload

        current = data.get("current", {})
        cur_temp = current.get("temperature_2m", 29.0)
        cur_wcode = current.get("weather_code", 1)
        cur_meta = map_wmo_weather_code(cur_wcode)

        hourly_data = data.get("hourly", {})
        hourly_times = hourly_data.get("time", [])
        hourly_probs = hourly_data.get("precipitation_probability", [])
        hourly_temps = hourly_data.get("temperature_2m", [])
        hourly_codes = hourly_data.get("weather_code", [])

        # Find current hour in times list
        cur_iso_prefix = now.strftime("%Y-%m-%dT%H:00")
        start_idx = 0
        for idx, t_str in enumerate(hourly_times):
            if t_str >= cur_iso_prefix:
                start_idx = idx
                break

        # Collect next 6 hours
        hourly_slots = []
        for i in range(start_idx, min(start_idx + 6, len(hourly_times))):
            t_str = hourly_times[i]
            # Parse hour like "15:00"
            hour_display = t_str.split("T")[1][:5] if "T" in t_str else t_str
            prob = hourly_probs[i] if i < len(hourly_probs) and hourly_probs[i] is not None else 0
            temp_val = hourly_temps[i] if i < len(hourly_temps) else cur_temp
            code_val = hourly_codes[i] if i < len(hourly_codes) else 0
            w_meta = map_wmo_weather_code(code_val)

            hourly_slots.append({
                "time": hour_display,
                "condition": w_meta["condition"],
                "icon": w_meta["icon"],
                "rain_prob": int(prob),
                "temp": round(float(temp_val), 1)
            })

        return {
            "location_name": loc_info["location_name"],
            "location_line1": loc_info["location_line1"],
            "location_line2": loc_info["location_line2"],
            "date_en": date_info["date_en"],
            "date_th": date_info["date_th"],
            "current_temp": round(float(cur_temp), 1),
            "current_condition": cur_meta["name_th"],
            "current_icon": cur_meta["icon"],
            "hourly": hourly_slots
        }

    except Exception as e:
        logger.error(f"Error fetching Open-Meteo forecast: {e}")
        return fallback_payload

