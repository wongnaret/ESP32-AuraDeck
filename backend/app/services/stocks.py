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
        "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36",
        "Accept": "application/json, text/plain, */*",
        "Accept-Language": "en-US,en;q=0.9",
        "Accept-Encoding": "gzip, deflate, br",
        "Referer": "https://finance.yahoo.com/",
        "Origin": "https://finance.yahoo.com",
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

                        # Clean symbol for UI display (e.g. CPALL.BK -> CPALL, BTC-USD -> BTC/USD, GC=F -> GOLD)
                        clean_symbol = symbol.split(".")[0]  # strip exchange suffix (.BK)
                        clean_symbol = clean_symbol.split("=")[0]  # strip futures suffix (=F)
                        if "-" in clean_symbol:
                            clean_symbol = clean_symbol.replace("-", "/")
                            
                        # Infer asset type
                        asset_type = "GLOBAL"
                        if ".BK" in symbol:
                            asset_type = "TH_STOCK"
                        elif "USD" in symbol or "THB" in symbol:
                            asset_type = "CRYPTO"
                        elif "=F" in symbol:  # Futures contracts (GC=F, CL=F etc.)
                            asset_type = "COMMODITY"
                            
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
    Fetches Thai gold bar price (96.5%) from GoldTraders API or HTML fallback,
    or falls back to Yahoo Finance Gold Futures (GC=F) if unavailable.
    """
    # Try GoldTraders JSON API first (more reliable than scraping)
    api_urls = [
        "https://www.goldtraders.or.th/api/Price",
        "https://www.goldtraders.or.th/",
    ]
    headers = {
        "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36",
        "Accept": "application/json, text/html, */*",
    }
    
    try:
        async with httpx.AsyncClient(follow_redirects=True) as client:
            # Attempt JSON API endpoint first
            try:
                api_response = await client.get(api_urls[0], headers=headers, timeout=5.0)
                if api_response.status_code == 200:
                    data = api_response.json()
                    # GoldTraders API returns {sell: ..., buy: ...} or [{name:..., sell:...}] etc.
                    sell_price = None
                    if isinstance(data, dict):
                        sell_price = data.get("sell") or data.get("Sell") or data.get("price") or data.get("Price")
                    elif isinstance(data, list) and len(data) > 0:
                        sell_price = data[0].get("sell") or data[0].get("Sell")
                    if sell_price and isinstance(sell_price, (int, float)) and 30000 < float(sell_price) < 80000:
                        return {
                            "symbol": "GOLD/TH",
                            "raw_symbol": "GOLD/TH",
                            "name": "Thai Gold Bar 96.5%",
                            "price": float(sell_price),
                            "change_pct": 0.0,
                            "type": "GOLD"
                        }
            except Exception:
                pass  # Fall through to HTML scraping
            
            # HTML scraping fallback — match realistic Thai gold prices (40,000–65,000 THB range)
            response = await client.get(api_urls[1], headers=headers, timeout=5.0)
            if response.status_code == 200:
                html = response.text
                # Match 5-digit prices like 44,000 - 64,999 (realistic gold range in THB)
                prices = re.findall(r'\b([4-6]\d,\d{3})\b', html)
                if len(prices) >= 1:
                    sell_price = float(prices[0].replace(",", ""))
                    logger.info(f"GoldTraders HTML scraper: found gold price {sell_price} THB")
                    return {
                        "symbol": "GOLD/TH",
                        "raw_symbol": "GOLD/TH",
                        "name": "Thai Gold Bar 96.5%",
                        "price": sell_price,
                        "change_pct": 0.0,
                        "type": "GOLD"
                    }
                    
    except Exception as e:
        logger.warning(f"GoldTraders.or.th scraper failed ({e}), falling back to Yahoo Finance Gold Futures...")
        

    # Fallback to Gold Futures (GC=F) via Yahoo Finance
    yf_gold = await fetch_yahoo_finance_price("GC=F", override_name="Gold Futures")
    if yf_gold:
        yf_gold["symbol"] = "GOLD"
        yf_gold["type"] = "GOLD"
        return yf_gold

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

    # Track raw symbols already fetched (from gold scraper) to avoid duplicates in watchlist loop
    fetched_raw_symbols = set()
    if gold_data:
        fetched_raw_symbols.add(gold_data.get("raw_symbol", ""))

    for item in watchlist_items:
        symbol = item.get("symbol")
        if not symbol or symbol == "GOLD/TH":
            continue
        # Skip if this raw_symbol was already fetched (e.g. GC=F already used as gold fallback)
        if symbol in fetched_raw_symbols:
            logger.debug(f"Skipping {symbol} — already fetched via gold scraper fallback.")
            continue
        # If name == symbol (auto-parsed from env, not a user-set display name),
        # pass None so fetch_yahoo_finance_price uses the proper longName from Yahoo
        override_name = item.get("name")
        if override_name == symbol:
            override_name = None
        asset_data = await fetch_yahoo_finance_price(symbol, override_name=override_name)
        if asset_data:
            fetched_raw_symbols.add(symbol)
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

