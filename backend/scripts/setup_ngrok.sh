#!/bin/bash

# setup_ngrok.sh - Auto installer and configurator for ngrok on Raspberry Pi (Debian/Ubuntu)
# Usage: ./setup_ngrok.sh [auth_token]

# Colors for pretty terminal output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}===================================================${NC}"
echo -e "${BLUE}      AuraDeck: ngrok Setup & Configuration        ${NC}"
echo -e "${BLUE}===================================================${NC}"

# Check if running on Linux
if [[ "$OSTYPE" != "linux-gnu"* ]]; then
    echo -e "${RED}Error: This script is intended to be run on Linux (Raspberry Pi / Debian / Ubuntu).${NC}"
    echo -e "If you are on Windows, please download and run ngrok manually from: https://ngrok.com/download"
    exit 1
fi

# 1. Install ngrok if not already installed
if ! command -v ngrok &> /dev/null; then
    echo -e "${YELLOW}[+] ngrok is not installed. Setting up official repository and installing...${NC}"
    
    # Ensure curl and ca-certificates are installed
    echo -e "${YELLOW}[+] Verifying dependencies (curl, ca-certificates)...${NC}"
    sudo apt update && sudo apt install -y curl ca-certificates
    
    # Clean up old invalid keys or repositories to prevent apt-update crashes
    echo -e "${YELLOW}[+] Cleaning up old ngrok key and repository configurations...${NC}"
    sudo rm -f /etc/apt/sources.list.d/ngrok.list
    sudo rm -f /usr/share/keyrings/ngrok.gpg
    sudo rm -f /etc/apt/trusted.gpg.d/ngrok.asc
    
    # Add ngrok's official ASCII armored GPG key and repository
    echo -e "${YELLOW}[+] Adding ngrok repository and official signing key (ngrok.asc)...${NC}"
    curl -sSL https://ngrok-agent.s3.amazonaws.com/ngrok.asc | sudo tee /etc/apt/trusted.gpg.d/ngrok.asc > /dev/null
    echo "deb https://ngrok-agent.s3.amazonaws.com buster main" | sudo tee /etc/apt/sources.list.d/ngrok.list
    
    # Update package lists and install ngrok
    echo -e "${YELLOW}[+] Installing ngrok...${NC}"
    sudo apt update && sudo apt install -y ngrok
    
    if command -v ngrok &> /dev/null; then
        echo -e "${GREEN}[✔] ngrok installed successfully!${NC}"
    else
        echo -e "${RED}[✗] Failed to install ngrok automatically. Please check your internet connection.${NC}"
        exit 1
    fi
else
    echo -e "${GREEN}[✔] ngrok is already installed on this machine: $(ngrok --version)${NC}"
fi

# 2. Get auth token
TOKEN=$1

# If no token passed as argument, try to read from local .env files
if [ -z "$TOKEN" ]; then
    ENV_PATH=""
    if [ -f ".env" ]; then
        ENV_PATH=".env"
    elif [ -f "backend/.env" ]; then
        ENV_PATH="backend/.env"
    elif [ -f "../.env" ]; then
        ENV_PATH="../.env"
    fi

    if [ -n "$ENV_PATH" ]; then
        # Parse NGROK_AUTHTOKEN from the .env file, removing carriage returns and quotes
        PARSED_TOKEN=$(grep -E "^NGROK_AUTHTOKEN=" "$ENV_PATH" | cut -d'=' -f2- | tr -d '\r' | tr -d '"' | tr -d "'")
        if [ -n "$PARSED_TOKEN" ] && [ "$PARSED_TOKEN" != "your_ngrok_authtoken" ]; then
            TOKEN=$PARSED_TOKEN
            echo -e "${GREEN}[✔] Loaded NGROK_AUTHTOKEN from $ENV_PATH${NC}"
        fi
    fi
fi

if [ -z "$TOKEN" ]; then
    echo -e "${YELLOW}No auth token provided as argument and no valid token found in .env files.${NC}"
    read -p "Please enter/paste your ngrok Auth Token: " TOKEN
fi

# Trim whitespace
TOKEN=$(echo "$TOKEN" | xargs)

if [ -z "$TOKEN" ]; then
    echo -e "${RED}Error: Auth Token cannot be empty. Setup aborted.${NC}"
    exit 1
fi

# 3. Configure Auth Token
echo -e "${YELLOW}[+] Authenticating your ngrok agent...${NC}"
ngrok config add-authtoken "$TOKEN"

if [ $? -eq 0 ]; then
    echo -e "${GREEN}[✔] ngrok token configured successfully!${NC}"
else
    echo -e "${RED}[✗] Failed to configure ngrok auth token.${NC}"
    exit 1
fi

# 4. Finished & Next Steps
echo -e "${BLUE}===================================================${NC}"
echo -e "${GREEN}             Setup Completed Successfully!          ${NC}"
echo -e "${BLUE}===================================================${NC}"
echo -e "To start your secure tunnel on port 8000, run:"
echo -e "  ${YELLOW}ngrok http 8000${NC}\n"
echo -e "Then copy the generated secure ${GREEN}https://xxxx.ngrok-free.app${NC} URL and update your ${YELLOW}.env${NC} file."
echo -e "Don't forget to register the URL in Google and Spotify developer portals!"
echo -e "${BLUE}===================================================${NC}"
