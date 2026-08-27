// =========================================================================
// Weather Map Location Selector (Leaflet + POI/ROI Geocoding + GPS)
// =========================================================================
let g_weatherMap = null;
let g_weatherMarker = null;
let g_mapSearchDebounceTimer = null;

function initWeatherMap(lat = 13.7563, lon = 100.5018, locationName = '') {
    if (g_weatherMap) return; // Prevent overwriting user's active pin during background polling

    const latNum = parseFloat(lat) || 13.7563;
    const lonNum = parseFloat(lon) || 100.5018;

    const mapContainer = document.getElementById('weather-map');
    if (!mapContainer || typeof L === 'undefined') return;

    if (locationName) {
        const locInput = document.getElementById('weather-location-name');
        if (locInput) locInput.value = locationName;
    }

    g_weatherMap = L.map('weather-map', {
        center: [latNum, lonNum],
        zoom: 12,
        zoomControl: true
    });

    // Clean Modern CartoDB Voyager Tile Layer
    L.tileLayer('https://{s}.basemaps.cartocdn.com/rastertiles/voyager/{z}/{x}/{y}{r}.png', {
        attribution: '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors &copy; <a href="https://carto.com/">CARTO</a>',
        subdomains: 'abcd',
        maxZoom: 19
    }).addTo(g_weatherMap);

    // Custom Glowing Aura Pin
    const pinIcon = L.divIcon({
        className: 'custom-weather-pin',
        html: '<div style="background: linear-gradient(135deg, #8b5cf6, #06b6d4); width: 30px; height: 30px; border-radius: 50% 50% 50% 0; transform: rotate(-45deg); border: 2px solid #ffffff; box-shadow: 0 0 14px rgba(139, 92, 246, 0.85); display: flex; align-items: center; justify-content: center; cursor: grab;"><span style="transform: rotate(45deg); font-size: 15px;">🌦️</span></div>',
        iconSize: [30, 30],
        iconAnchor: [15, 30],
        popupAnchor: [0, -30]
    });

    g_weatherMarker = L.marker([latNum, lonNum], {
        draggable: true,
        icon: pinIcon
    }).addTo(g_weatherMap);

    // Marker drag events
    g_weatherMarker.on('dragend', function(e) {
        const pos = e.target.getLatLng();
        setWeatherCoordinates(pos.lat, pos.lng, true);
    });

    // Map click anywhere to place marker
    g_weatherMap.on('click', function(e) {
        setWeatherCoordinates(e.latlng.lat, e.latlng.lng, true);
    });

    // Set initial coordinate fields & reverse-geocoded address
    setWeatherCoordinates(latNum, lonNum, !locationName);

    // Ensure proper render on load
    setTimeout(() => {
        if (g_weatherMap) g_weatherMap.invalidateSize();
    }, 350);
}

function setWeatherCoordinates(lat, lon, fetchReverse = true) {
    const latNum = parseFloat(lat);
    const lonNum = parseFloat(lon);
    const latFixed = latNum.toFixed(4);
    const lonFixed = lonNum.toFixed(4);

    const latInput = document.getElementById('weather-lat');
    const lonInput = document.getElementById('weather-lon');
    const coordsDisplay = document.getElementById('map-coords-display');

    if (latInput) latInput.value = latFixed;
    if (lonInput) lonInput.value = lonFixed;
    if (coordsDisplay) coordsDisplay.textContent = `${latFixed}, ${lonFixed}`;

    if (g_weatherMarker) {
        g_weatherMarker.setLatLng([latNum, lonNum]);
    }

    if (fetchReverse) {
        reverseGeocodeLocation(latNum, lonNum);
    }
}

async function reverseGeocodeLocation(lat, lon) {
    const nameLabel = document.getElementById('map-location-name');
    const locInput = document.getElementById('weather-location-name');
    if (nameLabel) nameLabel.textContent = '🔄 กำลังระบุชื่อสถานที่...';

    try {
        const url = `https://nominatim.openstreetmap.org/reverse?format=json&lat=${lat}&lon=${lon}&zoom=14&addressdetails=1`;
        const res = await fetch(url, { headers: { 'Accept-Language': 'th,en' } });
        if (res.ok) {
            const data = await res.json();
            const addr = data.address || {};
            const poi = data.name || addr.suburb || addr.neighbourhood || addr.amenity || addr.building || '';
            const district = addr.district || addr.subdistrict || addr.county || addr.city_district || '';
            const city = addr.city || addr.town || addr.province || addr.state || '';
            const country = addr.country || '';

            let displayName = [poi, district, city, country].filter(Boolean).join(', ');
            if (!displayName) displayName = data.display_name || `${lat.toFixed(4)}, ${lon.toFixed(4)}`;

            if (nameLabel) nameLabel.textContent = displayName;
            
            // Auto populate the input if empty or focused
            if (locInput && (!locInput.value || document.activeElement !== locInput)) {
                let cleanLine = [poi, district, city].filter(Boolean).join(', ') || displayName;
                locInput.value = cleanLine;
            }
        } else {
            if (nameLabel) nameLabel.textContent = `${lat.toFixed(4)}, ${lon.toFixed(4)}`;
        }
    } catch (e) {
        console.warn('Reverse geocode error:', e);
        if (nameLabel) nameLabel.textContent = `${parseFloat(lat).toFixed(4)}, ${parseFloat(lon).toFixed(4)}`;
    }
}

function onMapSearchInput(query) {
    clearTimeout(g_mapSearchDebounceTimer);
    const dropdown = document.getElementById('map-autocomplete-list');
    const q = query.trim();

    if (q.length < 2) {
        if (dropdown) dropdown.style.display = 'none';
        return;
    }

    g_mapSearchDebounceTimer = setTimeout(() => {
        searchPoiRoiLocation(q);
    }, 350);
}

async function searchPoiRoiLocation(query) {
    const dropdown = document.getElementById('map-autocomplete-list');
    if (!dropdown) return;

    dropdown.innerHTML = '<div class="map-autocomplete-item" style="color: var(--text-muted);">🔄 กำลังค้นหาสถานที่ POI/ROI...</div>';
    dropdown.style.display = 'block';

    try {
        const url = `https://nominatim.openstreetmap.org/search?format=json&q=${encodeURIComponent(query)}&addressdetails=1&limit=6`;
        const res = await fetch(url, { headers: { 'Accept-Language': 'th,en' } });
        if (!res.ok) throw new Error('Search failed');

        const results = await res.json();
        if (results.length === 0) {
            dropdown.innerHTML = '<div class="map-autocomplete-item" style="color: var(--text-muted);">❌ ไม่พบสถานที่ กรุณาลองค้นหาด้วยคำอื่น</div>';
            return;
        }

        dropdown.innerHTML = '';
        results.forEach(item => {
            const div = document.createElement('div');
            div.className = 'map-autocomplete-item';
            const icon = (item.type === 'city' || item.type === 'administrative') ? '🏙️' : '📍';
            div.innerHTML = `<span>${icon}</span> <div style="overflow: hidden; text-overflow: ellipsis; white-space: nowrap;"><strong>${item.name || item.display_name.split(',')[0]}</strong> <span style="font-size: 0.75rem; color: var(--text-muted); display: block;">${item.display_name}</span></div>`;
            div.onclick = () => {
                selectSearchResult(item);
            };
            dropdown.appendChild(div);
        });
    } catch (err) {
        console.error('POI search error:', err);
        dropdown.innerHTML = '<div class="map-autocomplete-item" style="color: var(--danger);">เกิดข้อผิดพลาดในการค้นหา</div>';
    }
}

function selectSearchResult(item) {
    const dropdown = document.getElementById('map-autocomplete-list');
    const searchInput = document.getElementById('map-search-input');
    const nameLabel = document.getElementById('map-location-name');
    const locInput = document.getElementById('weather-location-name');

    if (dropdown) dropdown.style.display = 'none';
    if (searchInput) searchInput.value = item.name || item.display_name.split(',')[0];
    if (nameLabel) nameLabel.textContent = item.display_name;

    const lat = parseFloat(item.lat);
    const lon = parseFloat(item.lon);

    if (locInput) {
        locInput.value = item.display_name;
    }

    if (g_weatherMap) {
        g_weatherMap.flyTo([lat, lon], 14, { duration: 1.2 });
    }
    setWeatherCoordinates(lat, lon, false);
}

function onMapSearchKeydown(event) {
    const dropdown = document.getElementById('map-autocomplete-list');
    if (event.key === 'Escape' && dropdown) {
        dropdown.style.display = 'none';
    }
}

// Close autocomplete dropdown when clicking outside
document.addEventListener('click', function(e) {
    const searchWrapper = document.querySelector('.map-search-wrapper');
    const dropdown = document.getElementById('map-autocomplete-list');
    if (dropdown && searchWrapper && !searchWrapper.contains(e.target)) {
        dropdown.style.display = 'none';
    }
});

function locateUserCurrentPosition() {
    if (!navigator.geolocation) {
        showToast('เบราว์เซอร์ไม่รองรับการระบุตำแหน่ง Geolocation', false);
        return;
    }

    showToast('กำลังค้นหาตำแหน่ง GPS ปัจจุบัน...');
    navigator.geolocation.getCurrentPosition(
        (pos) => {
            const lat = pos.coords.latitude;
            const lon = pos.coords.longitude;
            if (g_weatherMap) {
                g_weatherMap.flyTo([lat, lon], 14, { duration: 1.0 });
            }
            setWeatherCoordinates(lat, lon, true);
            showToast('ระบุตำแหน่งปัจจุบันสำเร็จแล้ว');
        },
        (err) => {
            console.error('Geolocation error:', err);
            showToast('ไม่สามารถเข้าถึงตำแหน่งได้: ' + err.message, false);
        },
        { enableHighAccuracy: true, timeout: 8000 }
    );
}

async function saveWeatherConfig() {
    const weather_lat_val = document.getElementById('weather-lat') ? parseFloat(document.getElementById('weather-lat').value) : 13.7563;
    const weather_lon_val = document.getElementById('weather-lon') ? parseFloat(document.getElementById('weather-lon').value) : 100.5018;
    const locInput = document.getElementById('weather-location-name');

    const payload = {
        weather_lat: isNaN(weather_lat_val) ? 13.7563 : weather_lat_val,
        weather_lon: isNaN(weather_lon_val) ? 100.5018 : weather_lon_val,
        weather_location_name: locInput ? locInput.value.trim() : ""
    };

    try {
        const response = await fetch(`/api/profiles/${profileId}/config`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(payload)
        });
        
        if (response.ok) {
            showToast("Weather location saved! Syncing weather to screen...");
            triggerSync('weather');
        } else {
            showToast("Failed to save weather location.", false);
        }
    } catch (err) {
        console.error(err);
        showToast("Server error saving weather config.", false);
    }
}

