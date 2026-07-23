import logging
import httpx
import re
from typing import List, Dict, Any, Optional

from app.config import settings

logger = logging.getLogger("stocks_service")

# Module level memory cache to preserve previous values on failure (Rule 3)
_last_known_prices: List[Dict[str, Any]] = []

DEFAULT_WATCHLIST: List[Dict[str, str]] = [
    {"symbol": "CPALL.BK", "name": "CP ALL Public Company Limited"},
    {"symbol": "BTC-USD", "name": "Bitcoin USD"},
    {"symbol": "GC=F", "name": "Gold Futures"}
]

async def search_stocks_yahoo(query: str) -> List[Dict[str, Any]]:
    """
    Searches Yahoo Finance autocomplete API for matching tickers/company names.
    Returns a list of dicts: [{"symbol": "...", "name": "...", "type": "...", "exchange": "..."}]
    """
    if not query or len(query.strip()) < 1:
        return []
        
    url = "https://query2.finance.yahoo.com/v1/finance/search"
    params = {
        "q": query.strip(),
        "quotesCount": 10,
        "newsCount": 0,
        "enableFuzzyQuery": "false",
        "quotesQueryId": "tss_match_phrase_query"
    }
    headers = {
        "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36"
    }
    
    try:
        async with httpx.AsyncClient() as client:
            response = await client.get(url, params=params, headers=headers, timeout=5.0)
            if response.status_code == 200:
                data = response.json()
                quotes = data.get("quotes", [])
                results = []
                for q in quotes:
                    symbol = q.get("symbol")
                    if not symbol:
                        continue
                    long_name = q.get("longname") or q.get("shortname") or symbol
                    quote_type = q.get("quoteType", "EQUITY")
                    exch = q.get("exchDisp") or q.get("exchange", "")
                    results.append({
                        "symbol": symbol,
                        "name": long_name,
                        "type": quote_type,
                        "exchange": exch
                    })
                return results
            logger.warning(f"Yahoo Finance search returned status {response.status_code} for '{query}'")
            return []
    except Exception as e:
        logger.error(f"Failed to search Yahoo Finance for '{query}': {e}")
        return []


async def fetch_yahoo_finance_price(symbol: str, override_name: Optional[str] = None) -> Dict[str, Any]:
    """
    Fetches the latest price and 24h change percentage for a given symbol from Yahoo Finance Chart API.
    Does not require any API keys.
    """
    url = f"https://query1.finance.yahoo.com/v8/finance/chart/{symbol}"
    params = {"interval": "1d", "range": "1d"}
    headers = {
        "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36"
    }
    
    try:
        async with httpx.AsyncClient() as client:
            response = await client.get(url, params=params, headers=headers, timeout=5.0)
            
            if response.status_code == 200:
                data = response.json()
                result = data.get("chart", {}).get("result", [])
                
                if result:
                    meta = result[0].get("meta", {})
                    price = meta.get("regularMarketPrice")
                    prev_close = meta.get("chartPreviousClose")
                    
                    if price is not None:
                        change_pct = 0.0
                        if prev_close and prev_close > 0:
                            change_pct = round(((price - prev_close) / prev_close) * 100, 2)
                        
                        full_name = override_name or meta.get("longName") or meta.get("shortName") or symbol

                        # Clean symbol for UI display (e.g. CPALL.BK -> CPALL, BTC-USD -> BTC/USD)
                        clean_symbol = symbol.split(".")[0]
                        if "-" in clean_symbol:
                            clean_symbol = clean_symbol.replace("-", "/")
                            
                        # Infer asset type
                        asset_type = "GLOBAL"
                        if ".BK" in symbol:
                            asset_type = "TH_STOCK"
                        elif "USD" in symbol or "THB" in symbol:
                            asset_type = "CRYPTO"
                            
                        return {
                            "symbol": clean_symbol,
                            "raw_symbol": symbol,
                            "name": full_name,
                            "price": round(price, 2),
                            "change_pct": change_pct,
                            "type": asset_type
                        }
            
            logger.warning(f"Yahoo Finance returned unexpected code {response.status_code} for {symbol}")
            return {}
            
    except Exception as e:
        logger.error(f"Failed to fetch Yahoo Finance for {symbol}: {e}")
        return {}


async def scrape_thai_gold_price() -> Dict[str, Any]:
    """
    Scrapes the official Gold Traders Association of Thailand website (goldtraders.or.th)
    for live Thai Gold bar prices using lightweight and robust regular expressions.
    """
    url = "https://www.goldtraders.or.th/"
    headers = {
        "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36"
    }
    
    try:
        async with httpx.AsyncClient() as client:
            response = await client.get(url, headers=headers, timeout=5.0)
            
            if response.status_code == 200:
                html = response.text
                
                # Regex match for buy and sell prices of Gold Bar (96.5%)
                buy_match = re.search(r'id="DetailPlace_uc_goldprices1_lblBLBuy"[^>]*>([\d,]+)</span>', html)
                sell_match = re.search(r'id="DetailPlace_uc_goldprices1_lblBLSell"[^>]*>([\d,]+)</span>', html)
                
                # Regex match for daily change (diff)
                diff_match = re.search(r'id="DetailPlace_uc_goldprices1_lblDiff"[^>]*>([^<]+)</span>', html)
                
                if buy_match and sell_match:
                    buy_price = float(buy_match.group(1).replace(",", ""))
                    sell_price = float(sell_match.group(1).replace(",", ""))
                    
                    diff_val = 0.0
                    if diff_match:
                        # Clean up HTML tags or text within diff
                        diff_text = diff_match.group(1)
                        # Extract any numbers with plus/minus signs
                        clean_diff = re.search(r'([+-]?\d+)', diff_text.replace(",", ""))
                        if clean_diff:
                            diff_val = float(clean_diff.group(1))
                            
                    # Calculate change percentage based on yesterday's closing price
                    change_pct = 0.0
                    prev_close = sell_price - diff_val
                    if prev_close > 0:
                        change_pct = round((diff_val / prev_close) * 100, 2)
                        
                    return {
                        "symbol": "GOLD/TH",
                        "raw_symbol": "GOLD/TH",
                        "name": "Thai Gold Bar 96.5%",
                        "price": sell_price, # We display the main sell price of Gold Bar
                        "change_pct": change_pct,
                        "type": "GOLD"
                    }
                    
            logger.warning(f"GoldTraders.or.th scraper returned status code: {response.status_code}")
            return {}
            
    except Exception as e:
        logger.error(f"Failed to scrape GoldTraders.or.th: {e}")
        return {}


async def get_multi_asset_prices(watchlist_items: Optional[List[Dict[str, str]]] = None) -> List[Dict[str, Any]]:
    """
    Aggregates gold prices from GoldTraders and stocks/cryptocurrencies from Yahoo Finance.
    Accepts custom watchlist_items list: [{"symbol": "...", "name": "..."}]
    Maintains a local cache to implement graceful degradation if any service fails (Rule 3).
    """
    global _last_known_prices
    
    aggregated_prices = []
    
    # 1. Fetch Thai Gold Prices
    gold_data = await scrape_thai_gold_price()
    if gold_data:
        aggregated_prices.append(gold_data)
        
    # 2. Determine watchlist items
    if not watchlist_items:
        # Fallback to settings or default list
        env_symbols = [s.strip() for s in settings.STOCK_WATCHLIST.split(",") if s.strip()]
        watchlist_items = [{"symbol": s, "name": s} for s in env_symbols] if env_symbols else DEFAULT_WATCHLIST

    for item in watchlist_items:
        symbol = item.get("symbol")
        if not symbol or symbol == "GOLD/TH":
            continue
        override_name = item.get("name")
        asset_data = await fetch_yahoo_finance_price(symbol, override_name=override_name)
        if asset_data:
            aggregated_prices.append(asset_data)
            
    # Graceful degradation logic (Rule 3)
    if aggregated_prices:
        # Update cache on success
        _last_known_prices = aggregated_prices
        return aggregated_prices
    else:
        logger.warning("All financial API requests failed. Returning last known cached financial state.")
        return _last_known_prices if _last_known_prices else [
            # Ultimate hardcoded fallback if everything fails and no cache exists on startup
            { "symbol": "GOLD/TH", "raw_symbol": "GOLD/TH", "name": "Thai Gold Bar 96.5%", "price": 41200.0, "change_pct": 0.0, "type": "GOLD" },
            { "symbol": "CPALL", "raw_symbol": "CPALL.BK", "name": "CP ALL Public Company Limited", "price": 57.25, "change_pct": 1.33, "type": "TH_STOCK" },
            { "symbol": "BTC/USD", "raw_symbol": "BTC-USD", "name": "Bitcoin USD", "price": 64500.0, "change_pct": 2.15, "type": "CRYPTO" }
        ]

