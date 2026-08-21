// =========================================================================
// GA4 Multi-Property Service Account Key Manager
// =========================================================================

// Fetch and render the list of uploaded GA4 properties with their Service Accounts
async function loadGa4Properties() {
    try {
        const response = await fetch(`/api/profiles/${profileId}/ga4-properties`);
        if (!response.ok) return;
        
        const properties = await response.json();
        const listContainer = document.getElementById('ga4-properties-list');
        if (!listContainer) return;
        
        if (properties.length === 0) {
            listContainer.innerHTML = `<div style="font-size: 0.85rem; color: var(--text-muted); text-align: center; padding: 6px;">No GA4 Properties added yet.</div>`;
            return;
        }
        
        listContainer.innerHTML = '';
        properties.forEach(p => {
            const row = document.createElement('div');
            row.style = 'display: flex; justify-content: space-between; align-items: center; padding: 10px 14px; background: rgba(255, 255, 255, 0.03); border: 1px solid rgba(255, 255, 255, 0.06); border-radius: 8px; margin-bottom: 6px;';
            row.innerHTML = `
                <div style="display: flex; flex-direction: column; gap: 2px; overflow: hidden; text-overflow: ellipsis; white-space: nowrap; max-width: 70%;">
                    <div style="display: flex; align-items: center; gap: 6px;">
                        <span style="font-weight: 600; font-size: 0.88rem; color: var(--text-primary);">📊 ${p.property_name}</span>
                        <span class="stock-symbol-badge" style="font-size: 0.72rem; padding: 1px 6px;">${p.property_id}</span>
                    </div>
                    <span style="font-size: 0.75rem; color: var(--text-muted); overflow: hidden; text-overflow: ellipsis; white-space: nowrap;">📧 SA: ${p.client_email}</span>
                </div>
                <div style="display: flex; gap: 6px;">
                    <button class="btn btn-secondary" style="padding: 4px 10px; font-size: 0.75rem; width: auto;" onclick="testGa4SingleProperty('${p.property_id}')">
                        🔍 Test
                    </button>
                    <button class="btn" style="padding: 4px 10px; font-size: 0.75rem; background: rgba(239, 68, 68, 0.2); border: 1px solid rgba(239, 68, 68, 0.3); color: #ef4444; width: auto;" 
                            onmouseover="this.style.background='rgba(239, 68, 68, 0.4)'" 
                            onmouseout="this.style.background='rgba(239, 68, 68, 0.2)'" 
                            onclick="deleteGa4PropertyKey('${p.property_id}')">
                        🗑️ Delete
                    </button>
                </div>
            `;
            listContainer.appendChild(row);
        });
    } catch (err) {
        console.error("Failed to load GA4 properties list:", err);
    }
}

// Upload a GA4 Service Account JSON key with Property ID and Display Name
async function uploadGa4PropertyKey(file) {
    if (!file) return;
    
    const propId = document.getElementById('ga4-upload-propid') ? document.getElementById('ga4-upload-propid').value.trim() : '';
    const propName = document.getElementById('ga4-upload-propname') ? document.getElementById('ga4-upload-propname').value.trim() : '';

    if (!propId) {
        showToast("Please enter the GA4 Property ID (e.g. 453120000) before uploading.", false);
        return;
    }

    const formData = new FormData();
    formData.append('property_id', propId);
    formData.append('property_name', propName);
    formData.append('file', file);
    
    showToast(`Uploading key for Property '${propName || propId}'...`);

    try {
        const response = await fetch(`/api/profiles/${profileId}/ga4-properties/upload`, {
            method: 'POST',
            body: formData
        });
        
        const data = await response.json();
        if (response.ok) {
            showToast(`Added GA4 Property '${data.property_name}' successfully!`);
            if (document.getElementById('ga4-upload-propid')) document.getElementById('ga4-upload-propid').value = '';
            if (document.getElementById('ga4-upload-propname')) document.getElementById('ga4-upload-propname').value = '';
            loadGa4Properties();
        } else {
            showToast(data.detail || "Failed to upload GA4 property key.", false);
        }
    } catch (err) {
        console.error("Upload error:", err);
        showToast("Server error during GA4 upload.", false);
    }
    
    const fileInput = document.getElementById('ga4-file-input');
    if (fileInput) fileInput.value = '';
}

// Delete an uploaded GA4 property
async function deleteGa4PropertyKey(propertyId) {
    if (!confirm(`Are you sure you want to remove GA4 Property '${propertyId}'?`)) return;
    
    try {
        const response = await fetch(`/api/profiles/${profileId}/ga4-properties/${propertyId}`, {
            method: 'DELETE'
        });
        
        const data = await response.json();
        if (response.ok) {
            showToast(`Removed GA4 Property '${propertyId}'.`);
            loadGa4Properties();
        } else {
            showToast(data.detail || "Failed to delete GA4 property key.", false);
        }
    } catch (err) {
        console.error("Deletion error:", err);
        showToast("Server error during deletion.", false);
    }
}

// Test a single GA4 property
async function testGa4SingleProperty(propertyId) {
    showToast(`Testing connection to GA4 Property ${propertyId}...`);
    try {
        const response = await fetch('/api/v1/ga4/test', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ property_id: propertyId, profile_id: profileId })
        });
        const res = await response.json();
        if (res.success) {
            showToast(`✅ ${res.message}`);
        } else {
            showToast(`❌ ${res.error}`, false);
        }
    } catch (e) {
        showToast(`❌ Test failed: ${e}`, false);
    }
}
