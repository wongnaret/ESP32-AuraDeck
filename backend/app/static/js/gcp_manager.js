// =========================================================================
// GCP Multi-Project Service Account Key Manager
// =========================================================================

// Fetch and render the list of uploaded GCP projects with their Service Accounts
async function loadGcpProjects() {
    try {
        const response = await fetch(`/api/profiles/${profileId}/gcp-projects`);
        if (!response.ok) return;
        
        const projects = await response.json();
        const listContainer = document.getElementById('gcp-projects-list');
        if (!listContainer) return;
        
        if (projects.length === 0) {
            listContainer.innerHTML = `<div style="font-size: 0.85rem; color: var(--text-muted); text-align: center; padding: 6px;">No GCP Projects added yet.</div>`;
            return;
        }
        
        listContainer.innerHTML = '';
        projects.forEach(p => {
            const projectRow = document.createElement('div');
            projectRow.style = 'display: flex; justify-content: space-between; align-items: center; padding: 10px 14px; background: rgba(255, 255, 255, 0.03); border: 1px solid rgba(255, 255, 255, 0.06); border-radius: 8px; margin-bottom: 6px;';
            const currTag = p.currency || 'THB';
            projectRow.innerHTML = `
                <div style="display: flex; flex-direction: column; gap: 2px; overflow: hidden; text-overflow: ellipsis; white-space: nowrap; max-width: 75%;">
                    <div style="display: flex; align-items: center; gap: 6px;">
                        <span style="font-weight: 600; font-size: 0.88rem; color: var(--text-primary);">📁 ${p.project_name || p.project_id}</span>
                        <span class="stock-type-tag" style="font-size: 0.7rem; padding: 1px 6px;">${currTag}</span>
                    </div>
                    <span style="font-size: 0.75rem; color: var(--text-muted); overflow: hidden; text-overflow: ellipsis; white-space: nowrap;">ID: <code>${p.project_id}</code> | SA: ${p.client_email}</span>
                </div>
                <button class="btn btn-primary" style="padding: 4px 10px; font-size: 0.75rem; background: rgba(239, 68, 68, 0.2); border-color: rgba(239, 68, 68, 0.3); color: #ef4444; width: auto;" 
                        onmouseover="this.style.background='rgba(239, 68, 68, 0.4)'" 
                        onmouseout="this.style.background='rgba(239, 68, 68, 0.2)'" 
                        onclick="deleteGcpProjectKey('${p.project_id}')">
                    🗑️ Delete
                </button>
            `;
            listContainer.appendChild(projectRow);
        });
    } catch (err) {
        console.error("Failed to load GCP projects list:", err);
    }
}

// Upload a GCP service account JSON key file with optional name and currency
async function uploadGcpProjectKey(file) {
    if (!file) return;
    
    const projName = document.getElementById('gcp-upload-name') ? document.getElementById('gcp-upload-name').value.trim() : '';
    const currency = document.getElementById('gcp-upload-currency') ? document.getElementById('gcp-upload-currency').value.trim() : 'THB';

    const formData = new FormData();
    formData.append('project_name', projName);
    formData.append('currency', currency);
    formData.append('file', file);
    
    showToast(`Uploading GCP Project key...`);

    try {
        const response = await fetch(`/api/profiles/${profileId}/gcp-projects/upload`, {
            method: 'POST',
            body: formData
        });
        
        const data = await response.json();
        if (response.ok) {
            showToast(`Added GCP Project '${data.project_name || data.project_id}' successfully!`);
            if (document.getElementById('gcp-upload-name')) document.getElementById('gcp-upload-name').value = '';
            loadGcpProjects();
        } else {
            showToast(data.detail || "Failed to upload GCP project key.", false);
        }
    } catch (err) {
        console.error("Upload error:", err);
        showToast("Server error during GCP upload.", false);
    }
    
    const fileInput = document.getElementById('gcp-file-input');
    if (fileInput) fileInput.value = '';
}

// Delete an uploaded GCP project
async function deleteGcpProjectKey(projectId) {
    if (!confirm(`Are you sure you want to remove GCP Project '${projectId}'?`)) return;
    
    try {
        const response = await fetch(`/api/profiles/${profileId}/gcp-projects/${projectId}`, {
            method: 'DELETE'
        });
        
        const data = await response.json();
        if (response.ok) {
            showToast(`Removed GCP Project '${projectId}'.`);
            loadGcpProjects();
        } else {
            showToast(data.detail || "Failed to delete project key.", false);
        }
    } catch (err) {
        console.error("Deletion error:", err);
        showToast("Server error during deletion.", false);
    }
}
