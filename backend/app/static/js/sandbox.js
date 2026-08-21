// =========================================================================
// Developer Sandbox, Mock MQTT Publisher & Terminal Console
// =========================================================================

// Console logger
function writeToConsole(message, type = 'info') {
    const body = document.getElementById('terminal-body');
    if (!body) return;
    const now = new Date().toLocaleTimeString();
    const line = document.createElement('div');
    line.className = `console-line ${type}`;
    line.innerHTML = `<span class="console-line time">[${now}]</span> ${message}`;
    body.appendChild(line);
    body.scrollTop = body.scrollHeight;
}

function clearConsole() {
    const body = document.getElementById('terminal-body');
    if (body) {
        body.innerHTML = '<div class="console-line info">[SYSTEM] Console cleared.</div>';
    }
}

// Trigger manual REST endpoint fetches and output response payload to console
async function triggerSync(service) {
    writeToConsole(`Triggering live REST synchronization for: <strong>${service}</strong>...`, 'info');
    try {
        const response = await fetch(`/api/sync/${service}`, { method: 'POST' });
        const data = await response.json();
        if (response.status === 200) {
            writeToConsole(`Live Sync SUCCESS for <strong>${service}</strong>. Response:`, 'success');
            writeToConsole(JSON.stringify(data, null, 2), 'success');
            showToast(`${service.toUpperCase()} sync successfully triggered.`);
        } else {
            writeToConsole(`Live Sync FAILED: ${data.detail || JSON.stringify(data)}`, 'error');
            showToast(`Failed to sync ${service}`, false);
        }
    } catch(e) {
        writeToConsole(`Request exception: ${e}`, 'error');
        showToast(`Request error: ${e}`, false);
    }
}

// Mocks definition dictionary
const mockTemplates = {
    "auradeck/spotify": {
        "is_playing": true,
        "title": "เพลงรักในสายลม",
        "artist": "วงดนตรีสากล",
        "album": "บทเพลงแห่งความคิดถึง",
        "progress_ms": 128000,
        "duration_ms": 240000,
        "has_lyrics": true,
        "current_lyric": "อยากบอกให้เธอรู้ ว่าฉันรักเธอเท่าไหร่...",
        "next_lyric": "แม้เวลาจะหมุนเวียนเปลี่ยนไปนานแค่ไหน..."
    },
    "auradeck/calendar": {
        "month_days_with_events": [1, 5, 12, 15, 20, 21, 22, 28],
        "events": [
            { "time": "14:00", "title": "ประชุมทีมสถาปัตยกรรม", "is_today": true },
            { "time": "Tomorrow 10:00", "title": "สแตนด์อัปรายวัน", "is_today": false }
        ]
    },
    "auradeck/todos": [
        { "id": "1", "title": "ตรวจทาน Pull Request #42", "completed": false },
        { "id": "2", "title": "ติดตั้งโปรแกรมปรับปรุงระบบฐานข้อมูล", "completed": false }
    ],
    "auradeck/stocks": [
        { "symbol": "CPALL", "price": 57.25, "change_pct": 1.33, "type": "TH_STOCK" },
        { "symbol": "BTC/THB", "price": 2350000.00, "change_pct": 2.15, "type": "CRYPTO" },
        { "symbol": "GOLD_TH", "price": 41200.00, "change_pct": -0.24, "type": "GOLD" }
    ],
    "auradeck/antigravity": {
        "limit_5h": { "used": 12.5, "total": 50.0, "percentage": 25.0 },
        "limit_weekly": { "used": 140.0, "total": 500.0, "percentage": 28.0 },
        "next_reset": "02h 15m"
    },
    "auradeck/ga4": {
        "active_users_30m": 42,
        "active_28d_users": "18.5K",
        "new_users": "2.4K",
        "avg_engagement_time": "2m 35s",
        "event_count": "112.8K",
        "top_cities": [
            { "city": "Bangkok", "active_users": 22 },
            { "city": "Chiang Mai", "active_users": 8 },
            { "city": "Nonthaburi", "active_users": 5 },
            { "city": "Phuket", "active_users": 4 },
            { "city": "Chon Buri", "active_users": 3 }
        ]
    },
    "auradeck/gcp": {
        "total_projects": 2,
        "projects": [
            {
                "project_id": "auradeck-prod",
                "project_name": "AuraDeck Prod",
                "currency": "THB",
                "cost_mtd": 14250.00,
                "forecast_end_of_month": 18500.00,
                "service_breakdown": [
                    { "service": "Compute Engine", "cost": 6412.50, "pct": 45 },
                    { "service": "BigQuery & AI", "cost": 3562.50, "pct": 25 },
                    { "service": "Cloud Run / GKE", "cost": 2850.00, "pct": 20 },
                    { "service": "Cloud Storage", "cost": 1425.00, "pct": 10 }
                ],
                "daily_costs": [
                    { "date": "11/08", "cost": 420.0 },
                    { "date": "12/08", "cost": 450.0 },
                    { "date": "13/08", "cost": 480.0 },
                    { "date": "14/08", "cost": 510.0 },
                    { "date": "15/08", "cost": 490.0 },
                    { "date": "16/08", "cost": 460.0 },
                    { "date": "17/08", "cost": 530.0 },
                    { "date": "18/08", "cost": 550.0 },
                    { "date": "19/08", "cost": 520.0 },
                    { "date": "20/08", "cost": 540.0 }
                ]
            }
        ]
    },
    "auradeck/weather": {
        "date_en": "Thursday, 20 August 2026",
        "date_th": "วันพฤหัสบดีที่ 20 สิงหาคม 2569",
        "current_temp": 28.5,
        "current_condition": "มีเมฆมาก",
        "current_icon": "CLOUD",
        "hourly": [
            { "time": "18:00", "condition": "Drizzle", "icon": "DRIZZLE", "rain_prob": 80, "temp": 28.0 },
            { "time": "19:00", "condition": "Rain", "icon": "RAIN", "rain_prob": 85, "temp": 27.5 },
            { "time": "20:00", "condition": "Drizzle", "icon": "DRIZZLE", "rain_prob": 70, "temp": 27.0 },
            { "time": "21:00", "condition": "Cloudy", "icon": "CLOUD", "rain_prob": 30, "temp": 26.8 },
            { "time": "22:00", "condition": "Cloudy", "icon": "CLOUD", "rain_prob": 20, "temp": 26.5 },
            { "time": "23:00", "condition": "Clear", "icon": "SUN", "rain_prob": 10, "temp": 26.2 }
        ]
    }
};

async function loadMockTemplate() {
    const topic = document.getElementById('mqtt-topic').value;
    if (topic === 'auradeck/stocks' && typeof profileId !== 'undefined' && profileId) {
        try {
            const response = await fetch(`/api/v1/stocks/watchlist?profile_id=${profileId}`);
            if (response.ok) {
                const data = await response.json();
                if (data.items && data.items.length > 0) {
                    document.getElementById('mqtt-payload').value = JSON.stringify(data.items, null, 4);
                    return;
                }
            }
        } catch (e) {
            console.warn("Could not load live stock watchlist for sandbox:", e);
        }
    }
    const template = mockTemplates[topic];
    document.getElementById('mqtt-payload').value = JSON.stringify(template, null, 4);
}

// Send mock publish events to backend
async function publishMqtt() {
    const topic = document.getElementById('mqtt-topic').value;
    const payloadStr = document.getElementById('mqtt-payload').value;
    
    try {
        const payload = JSON.parse(payloadStr);
        writeToConsole(`Publishing test payload to topic: <strong>${topic}</strong>...`, 'info');
        
        const response = await fetch('/api/publish', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ topic, payload })
        });
        
        const data = await response.json();
        if (response.status === 200) {
            writeToConsole(`MQTT Publish SUCCESS on <strong>${topic}</strong>. Payload:`, 'success');
            writeToConsole(JSON.stringify(payload), 'success');
            showToast(`Successfully published mock to ${topic}!`);
        } else {
            writeToConsole(`MQTT Publish FAILED: ${data.detail || JSON.stringify(data)}`, 'error');
            showToast(`Failed to publish message.`, false);
        }
    } catch(e) {
        writeToConsole(`Invalid JSON payload format: ${e.message}`, 'error');
        showToast(`JSON parsing error!`, false);
    }
}
