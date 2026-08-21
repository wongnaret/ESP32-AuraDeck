// =========================================================================
// AuraDeck Core App (Navigation, Config, Polling, Toast & Life-Cycle)
// =========================================================================

// Global Toast Display
function showToast(message, isSuccess = true) {
    const toast = document.getElementById('toast');
    const toastMsg = document.getElementById('toast-msg');
    if (!toast || !toastMsg) return;

    toastMsg.textContent = message;
    toast.style.background = isSuccess ? 'rgba(16, 185, 129, 0.95)' : 'rgba(239, 68, 68, 0.95)';
    toast.classList.add('show');
    setTimeout(() => {
        toast.classList.remove('show');
    }, 4000);
}

// Navigation Tabs
function switchTab(tabId, el) {
    document.querySelectorAll('.tab-content').forEach(t => t.classList.remove('active'));
    document.querySelectorAll('.nav-item').forEach(n => n.classList.remove('active'));
    
    const targetTab = document.getElementById(tabId);
    if (targetTab) targetTab.classList.add('active');
    if (el) el.classList.add('active');

    // Leaflet map needs recalculating dimensions when tab becomes visible
    if (tabId === 'integrations' && typeof g_weatherMap !== 'undefined' && g_weatherMap) {
        setTimeout(() => {
            g_weatherMap.invalidateSize();
        }, 200);
    }
}

// OAuth Authorize Trigger
function authorizeService(service) {
    window.location.href = `/auth/${service}`;
}

// Update Status Badges
function updateBadge(badgeId, btnId, isConnected) {
    const badge = document.getElementById(badgeId);
    const btn = document.getElementById(btnId);
    if (!badge || !btn) return;

    if (isConnected) {
        badge.className = "status-badge connected";
        badge.querySelector('.status-text').innerText = "Connected";
        btn.innerText = "Re-authorize Service";
        btn.className = "btn btn-secondary";
    } else {
        badge.className = "status-badge disconnected";
        badge.querySelector('.status-text').innerText = "Disconnected";
        btn.innerText = `Authorize ${btnId.includes('google') ? 'Google' : 'Spotify'}`;
        btn.className = "btn btn-primary";
    }
}

// Load Profile Configuration
async function loadProfileConfig() {
    try {
        const response = await fetch(`/api/profiles/${profileId}/config`);
        if (!response.ok) return;
        
        const data = await response.json();
        
        const profileLabel = document.getElementById('sidebar-profile-name');
        if (profileLabel) profileLabel.textContent = data.profile_name || profileId;
        
        // Prevent wiping user input while actively typing
        const gaInput = document.getElementById('ga-property-id');
        if (gaInput && document.activeElement !== gaInput) {
            gaInput.value = data.ga_property_id || '';
        }
        
        // Initialize weather map if first load
        const wLat = data.weather_lat !== undefined ? data.weather_lat : 13.7563;
        const wLon = data.weather_lon !== undefined ? data.weather_lon : 100.5018;
        if (typeof initWeatherMap === 'function') {
            initWeatherMap(wLat, wLon);
        }
        
        // Fetch dynamic active credential state
        const stateResponse = await fetch('/api/status');
        const stateData = await stateResponse.json();
        
        updateBadge('google-status', 'google-btn', stateData.google);
        updateBadge('spotify-status', 'spotify-btn', stateData.spotify);
        
        const taskCard = document.getElementById('task-lists-card');
        if (stateData.google) {
            if (taskCard) taskCard.style.display = 'block';
            loadGoogleTaskLists(data.active_task_lists);
        } else {
            if (taskCard) taskCard.style.display = 'none';
        }

        // Populate Polling Intervals dropdowns if present
        if (data.polling_intervals) {
            const pi = data.polling_intervals;
            const setVal = (id, val) => {
                const el = document.getElementById(id);
                if (el && val) el.value = val;
            };
            setVal('interval-tasks-calendar', pi.tasks_calendar_mins);
            setVal('interval-weather', pi.weather_mins);
            setVal('interval-stocks', pi.stocks_mins);
            setVal('interval-antigravity', pi.antigravity_mins);
            setVal('interval-ga4', pi.ga4_mins);
            setVal('interval-gcp', pi.gcp_mins);
        }
    } catch (err) {
        console.error("Failed to load profile configuration: ", err);
    }
}

// Google Analytics 4 Test Connection
async function testGa4Connection() {
    const propId = document.getElementById('ga-property-id').value.trim();
    const resultDiv = document.getElementById('ga4-test-result');
    if (!propId) {
        resultDiv.style.display = 'block';
        resultDiv.style.color = '#ef4444';
        resultDiv.textContent = '❌ Please enter a GA4 Property ID first.';
        return;
    }

    resultDiv.style.display = 'block';
    resultDiv.style.color = '#60a5fa';
    resultDiv.textContent = '🔄 Testing connection to GA4 Property...';

    try {
        const response = await fetch('/api/v1/ga4/test', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ property_id: propId, profile_id: profileId })
        });
        const res = await response.json();
        if (res.success) {
            resultDiv.style.color = '#10b981';
            resultDiv.textContent = `✅ ${res.message}`;
        } else {
            resultDiv.style.color = '#ef4444';
            resultDiv.textContent = `❌ ${res.error}`;
        }
    } catch (e) {
        resultDiv.style.color = '#ef4444';
        resultDiv.textContent = `❌ Test request failed: ${e}`;
    }
}

// Save GA4 Settings Independently
async function saveGa4Config() {
    const ga_property_id = document.getElementById('ga-property-id').value.trim();
    const payload = { ga_property_id };

    try {
        const response = await fetch(`/api/profiles/${profileId}/config`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(payload)
        });
        
        if (response.ok) {
            showToast("GA4 Property settings saved successfully!");
        } else {
            showToast("Failed to save GA4 configuration.", false);
        }
    } catch (err) {
        console.error(err);
        showToast("Server error saving GA4 config.", false);
    }
}

// Save Polling Intervals
async function savePollingIntervals() {
    const tasks_calendar_mins = parseInt(document.getElementById('interval-tasks-calendar').value, 10);
    const weather_mins = parseInt(document.getElementById('interval-weather').value, 10);
    const stocks_mins = parseInt(document.getElementById('interval-stocks').value, 10);
    const antigravity_mins = parseInt(document.getElementById('interval-antigravity').value, 10);
    const ga4_mins = parseInt(document.getElementById('interval-ga4').value, 10);
    const gcp_mins = parseInt(document.getElementById('interval-gcp').value, 10);

    const payload = {
        profile_id: profileId,
        tasks_calendar_mins,
        weather_mins,
        stocks_mins,
        antigravity_mins,
        ga4_mins,
        gcp_mins
    };

    showToast("Saving intervals & triggering screen sync...");

    try {
        const response = await fetch('/api/v1/profile/intervals', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(payload)
        });
        const data = await response.json();
        if (response.ok) {
            showToast(data.message || "Sync intervals updated successfully!");
        } else {
            showToast(data.detail || "Failed to update intervals.", false);
        }
    } catch (err) {
        console.error(err);
        showToast("Network error updating intervals.", false);
    }
}

// Google Task Lists Management
async function loadGoogleTaskLists(selectedLists) {
    const container = document.getElementById('task-lists-container');
    if (!container) return;

    try {
        const response = await fetch(`/api/profiles/${profileId}/google-lists`);
        if (!response.ok) {
            container.innerHTML = `<div style="font-size: 0.9rem; color: var(--danger);">Google API query failed. Try re-authorizing Google.</div>`;
            return;
        }
        const data = await response.json();
        
        if (data.length === 0) {
            container.innerHTML = `<div style="font-size: 0.85rem; color: var(--text-muted);">No task lists found on this Google account.</div>`;
            return;
        }

        container.innerHTML = '';
        data.forEach(lst => {
            const isChecked = selectedLists && selectedLists.includes(lst.id);
            const item = document.createElement('label');
            item.className = 'checkbox-item';
            item.innerHTML = `
                <input type="checkbox" value="${lst.id}" ${isChecked ? 'checked' : ''} onchange="updateSelectedTaskLists()">
                <span>${lst.title}</span>
            `;
            container.appendChild(item);
        });
    } catch (err) {
        console.error(err);
        container.innerHTML = `<div style="font-size: 0.9rem; color: var(--danger);">Error fetching list data.</div>`;
    }
}

async function updateSelectedTaskLists() {
    const checkboxes = document.querySelectorAll('#task-lists-container input[type="checkbox"]');
    const selected = [];
    checkboxes.forEach(cb => {
        if (cb.checked) selected.push(cb.value);
    });

    try {
        await fetch(`/api/profiles/${profileId}/config`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ active_task_lists: selected })
        });
        showToast("Task lists selection saved.");
    } catch (err) {
        console.error(err);
    }
}

// Initial Dashboard Loading Routine
async function initializeDashboard() {
    await loadProfileConfig();
    if (typeof loadGcpProjects === 'function') await loadGcpProjects();
    if (typeof loadPairedDevices === 'function') await loadPairedDevices();
    if (typeof fetchAPStatus === 'function') await fetchAPStatus();
    if (typeof loadStockWatchlist === 'function') await loadStockWatchlist();
}

// Launch on page ready
document.addEventListener('DOMContentLoaded', () => {
    initializeDashboard();
    
    // Polling states periodically (Rule 5)
    setInterval(loadProfileConfig, 15000);
    if (typeof loadPairedDevices === 'function') setInterval(loadPairedDevices, 15000);
    if (typeof fetchAPStatus === 'function') setInterval(fetchAPStatus, 15000);
});
