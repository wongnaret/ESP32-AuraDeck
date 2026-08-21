// =========================================================================
// Device Pairing & Local Hotspot AP Management
// =========================================================================

// Device Pairing Management
async function loadPairedDevices() {
    const container = document.getElementById('paired-devices-container');
    try {
        const response = await fetch('/api/pairing/list');
        if (!response.ok) return;
        
        const devices = await response.json();
        if (devices.length === 0) {
            container.innerHTML = `<div style="font-size: 0.85rem; color: var(--text-muted);">No screens paired yet.</div>`;
            return;
        }
        
        container.innerHTML = '';
        devices.forEach(dev => {
            const item = document.createElement('div');
            item.className = 'paired-item';
            item.innerHTML = `
                <div>
                    <span class="paired-mac">${dev.mac}</span>
                    <div style="font-size: 0.75rem; color: var(--text-muted); margin-top: 2px;">Paired: ${dev.paired_at ? new Date(dev.paired_at).toLocaleTimeString() : 'Active'}</div>
                </div>
                <button class="btn btn-danger" onclick="unpairDevice('${dev.mac}')" style="width: auto; padding: 6px 12px; font-size: 0.75rem;">
                    Unpair
                </button>
            `;
            container.appendChild(item);
        });
    } catch (err) {
        console.error(err);
    }
}

async function pairDevice() {
    const pin = document.getElementById('pair-pin').value.trim();
    if (pin.length !== 6) {
        showToast("Please enter a valid 6-digit PIN code displayed on screen.", false);
        return;
    }

    try {
        const response = await fetch('/api/pairing/verify', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ pin: pin })
        });
        
        const data = await response.json();
        if (response.ok) {
            showToast(`Screen (${data.mac}) paired successfully!`);
            document.getElementById('pair-pin').value = '';
            loadPairedDevices();
        } else {
            showToast(data.detail || "Pairing failed. Invalid or expired PIN.", false);
        }
    } catch (err) {
        console.error(err);
        showToast("Server error during pairing.", false);
    }
}

async function unpairDevice(mac) {
    if (!confirm(`Unpair screen ${mac}?`)) return;

    try {
        const response = await fetch('/api/pairing/unpair', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ mac: mac })
        });
        
        if (response.ok) {
            showToast(`Unpaired ${mac}`);
            loadPairedDevices();
        } else {
            showToast("Failed to unpair device.", false);
        }
    } catch (err) {
        console.error(err);
    }
}

// Hotspot Access Point Status & Management
async function fetchAPStatus() {
    try {
        const response = await fetch('/api/ap/status');
        if (!response.ok) return;
        const data = await response.json();
        
        const badge = document.getElementById('ap-status-badge');
        const statusText = document.getElementById('ap-status-text');
        
        if (data.status === "ACTIVE") {
            badge.className = "status-badge active-pulse";
            statusText.textContent = "Active";
        } else {
            badge.className = "status-badge disconnected";
            statusText.textContent = "Disabled";
        }

        document.getElementById('ap-ssid').textContent = data.ssid;
        document.getElementById('ap-password').textContent = data.password;
        document.getElementById('ap-gateway').textContent = data.gateway;
        document.getElementById('ap-clients').textContent = `${data.clients_connected} Devices`;
    } catch (err) {
        console.error(err);
    }
}

async function restartHotspot() {
    const btn = document.getElementById('ap-restart-btn');
    btn.disabled = true;
    btn.textContent = "Restarting Hotspot AP...";
    showToast("Rebooting hotspot access point connection. Standby...");

    try {
        const response = await fetch('/api/ap/restart', { method: 'POST' });
        const data = await response.json();
        if (response.ok) {
            showToast(data.message);
        } else {
            showToast("Failed to restart hotspot.", false);
        }
    } catch (err) {
        console.error(err);
        showToast("Connection timed out. Hotspot restarting.", true);
    } finally {
        setTimeout(() => {
            btn.disabled = false;
            btn.textContent = "🔄 Restart Local Hotspot AP";
            fetchAPStatus();
        }, 3000);
    }
}
