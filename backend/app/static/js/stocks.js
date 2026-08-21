// =========================================================================
// Stock & Asset Watchlist Management (Yahoo Finance Search & CRUD)
// =========================================================================
let selectedStockSymbol = "";
let selectedStockName = "";
let stockSearchDebounceTimer = null;

async function loadStockWatchlist() {
    try {
        const response = await fetch(`/api/v1/stocks/watchlist?profile_id=${profileId}`);
        if (!response.ok) return;
        const data = await response.json();
        
        const tbody = document.getElementById('stock-watchlist-table-body');
        tbody.innerHTML = '';

        const items = data.items || [];
        if (items.length === 0) {
            tbody.innerHTML = `<tr><td colspan="5" style="text-align: center; padding: 16px; color: var(--text-muted);">No stocks in watchlist. Add one above!</td></tr>`;
            return;
        }

        items.forEach(item => {
            const tr = document.createElement('tr');
            
            const symbol = item.symbol || item.raw_symbol || 'N/A';
            const rawSymbol = item.raw_symbol || item.symbol || '';
            const name = item.name || symbol;
            const type = item.type || 'GLOBAL';
            const price = item.price !== undefined && item.price !== null ? item.price.toLocaleString(undefined, {minimumFractionDigits: 2, maximumFractionDigits: 2}) : 'N/A';
            const changePct = item.change_pct || 0;
            
            const changeColor = changePct > 0 ? '#10b981' : (changePct < 0 ? '#ef4444' : '#9ca3af');
            const changeSign = changePct > 0 ? '+' : '';

            tr.innerHTML = `
                <td style="padding: 8px 6px;">
                    <span class="stock-symbol-badge">${symbol}</span>
                    ${rawSymbol && rawSymbol !== symbol ? `<div style="font-size: 0.75rem; color: var(--text-muted); margin-top: 2px;">${rawSymbol}</div>` : ''}
                </td>
                <td style="padding: 8px 6px; font-weight: 500; color: var(--text-main);">${name}</td>
                <td style="padding: 8px 6px;"><span class="stock-type-tag">${type}</span></td>
                <td style="padding: 8px 6px;">
                    <div style="font-weight: 600; font-family: var(--font-mono);">${price}</div>
                    <div style="font-size: 0.8rem; color: ${changeColor}; font-weight: 600;">${changeSign}${changePct}%</div>
                </td>
                <td style="padding: 8px 6px; text-align: right;">
                    <button class="btn-danger-sm" onclick="deleteStock('${rawSymbol || symbol}')">
                        🗑️ ลบ
                    </button>
                </td>
            `;
            tbody.appendChild(tr);
        });
    } catch (err) {
        console.error("Failed to load stock watchlist:", err);
    }
}

function onStockSearchInput(query) {
    clearTimeout(stockSearchDebounceTimer);
    const dropdown = document.getElementById('stock-autocomplete-dropdown');
    
    if (!query || query.trim().length < 1) {
        dropdown.style.display = 'none';
        dropdown.innerHTML = '';
        return;
    }

    stockSearchDebounceTimer = setTimeout(async () => {
        try {
            const response = await fetch(`/api/v1/stocks/search?q=${encodeURIComponent(query.trim())}`);
            if (!response.ok) return;
            const results = await response.json();
            
            dropdown.innerHTML = '';
            if (!results || results.length === 0) {
                dropdown.innerHTML = `<div style="padding: 12px; font-size: 0.85rem; color: var(--text-muted); text-align: center;">No matching stocks found</div>`;
                dropdown.style.display = 'block';
                return;
            }

            results.forEach(res => {
                const itemDiv = document.createElement('div');
                itemDiv.className = 'stock-autocomplete-item';
                
                const safeSymbol = res.symbol.replace(/'/g, "\\'");
                const safeName = (res.name || res.symbol).replace(/'/g, "\\'");
                
                itemDiv.innerHTML = `
                    <div>
                        <span class="stock-symbol-badge">${res.symbol}</span>
                        <span style="font-weight: 500; margin-left: 8px; font-size: 0.9rem;">${res.name}</span>
                    </div>
                    <div style="display: flex; gap: 6px; align-items: center;">
                        ${res.exchange ? `<span style="font-size: 0.75rem; color: var(--text-muted);">${res.exchange}</span>` : ''}
                        <span class="stock-type-tag">${res.type}</span>
                    </div>
                `;
                itemDiv.onclick = () => selectStockSuggestion(safeSymbol, safeName);
                dropdown.appendChild(itemDiv);
            });

            dropdown.style.display = 'block';
        } catch (err) {
            console.error("Error searching stocks:", err);
        }
    }, 300);
}

function selectStockSuggestion(symbol, name) {
    selectedStockSymbol = symbol;
    selectedStockName = name;
    const input = document.getElementById('stock-search-input');
    input.value = `${symbol} - ${name}`;
    document.getElementById('stock-autocomplete-dropdown').style.display = 'none';
}

async function addSelectedStock() {
    const inputVal = document.getElementById('stock-search-input').value.trim();
    if (!inputVal) {
        showToast("Please enter or select a stock symbol.", false);
        return;
    }

    let symbolToAdd = selectedStockSymbol;
    let nameToAdd = selectedStockName;

    if (!symbolToAdd || !inputVal.includes(symbolToAdd)) {
        // If user manually typed a raw ticker symbol like "NVDA" or "CPALL.BK"
        symbolToAdd = inputVal.split(' - ')[0].trim().toUpperCase();
        nameToAdd = inputVal.includes(' - ') ? inputVal.split(' - ')[1].trim() : symbolToAdd;
    }

    try {
        const response = await fetch('/api/v1/stocks/watchlist', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                symbol: symbolToAdd,
                name: nameToAdd,
                profile_id: profileId
            })
        });

        const data = await response.json();
        if (response.ok) {
            showToast(`Added ${symbolToAdd} to watchlist!`);
            document.getElementById('stock-search-input').value = '';
            selectedStockSymbol = '';
            selectedStockName = '';
            document.getElementById('stock-autocomplete-dropdown').style.display = 'none';
            loadStockWatchlist();
        } else {
            showToast(data.detail || "Failed to add stock.", false);
        }
    } catch (err) {
        console.error("Error adding stock:", err);
        showToast("Server error adding stock.", false);
    }
}

async function deleteStock(symbol) {
    try {
        const response = await fetch(`/api/v1/stocks/watchlist?symbol=${encodeURIComponent(symbol)}&profile_id=${profileId}`, {
            method: 'DELETE'
        });

        const data = await response.json();
        if (response.ok) {
            showToast(`Removed ${symbol} from watchlist.`);
            loadStockWatchlist();
        } else {
            showToast("Failed to delete stock.", false);
        }
    } catch (err) {
        console.error("Error deleting stock:", err);
        showToast("Server error deleting stock.", false);
    }
}

// Close stock dropdown when clicking outside
document.addEventListener('click', (e) => {
    const wrapper = document.querySelector('.stock-search-wrapper');
    if (wrapper && !wrapper.contains(e.target)) {
        const dropdown = document.getElementById('stock-autocomplete-dropdown');
        if (dropdown) dropdown.style.display = 'none';
    }
});
