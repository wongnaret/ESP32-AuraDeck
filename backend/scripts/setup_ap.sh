#!/bin/bash

# ==============================================================================
# AuraDeck Raspberry Pi Wi-Fi Access Point (AP) Hotspot Setup Script
# Script สำหรับตั้งค่าให้ Raspberry Pi เป็นตัวปล่อยสัญญาณ Wi-Fi สำหรับ ESP32-S3
# ==============================================================================

# Ensure the script is run with root privileges
if [ "$EUID" -ne 0 ]; then
  echo "❌ Error: Please run this script with sudo."
  echo "วิธีรัน: sudo ./setup_ap.sh"
  exit 1
fi

echo "=========================================================="
echo "🌀 Setting up AuraDeck Local Wi-Fi Access Point..."
echo "=========================================================="

# 1. Detect Network Manager (nmcli)
if ! command -v nmcli &> /dev/null; then
  echo "⚠️ NetworkManager is not installed or active."
  echo "Installing NetworkManager..."
  apt-get update && apt-get install -y network-manager
fi

# Ensure NetworkManager is running
systemctl start NetworkManager
systemctl enable NetworkManager

# 2. Check for wireless interface (usually wlan0)
WLAN_INTERFACE="wlan0"
if [ ! -d "/sys/class/net/$WLAN_INTERFACE" ]; then
  # Try to find any active wireless device
  WLAN_INTERFACE=$(nmcli device | grep wifi | awk '{print $1}' | head -n 1)
  if [ -z "$WLAN_INTERFACE" ]; then
    echo "❌ Error: No wireless interface (Wi-Fi hardware) detected!"
    exit 1
  fi
fi

echo "📶 Found Wi-Fi interface: $WLAN_INTERFACE"

# 3. Define Hotspot credentials
SSID="AuraDeck_AP"
PASSWORD="AuraDeck1234"
CON_NAME="AuraDeck_Hotspot"

# Delete existing connection if it exists to avoid conflicts
echo "🧹 Cleaning previous AuraDeck connection profiles..."
nmcli connection delete "$CON_NAME" &> /dev/null
nmcli connection delete "Hotspot" &> /dev/null

# 4. Create and Configure the Hotspot Connection
echo "⚙️ Creating new Hotspot connection profile..."
nmcli connection add \
  type wifi \
  ifname "$WLAN_INTERFACE" \
  con-name "$CON_NAME" \
  autoconnect yes \
  ssid "$SSID"

if [ $? -ne 0 ]; then
  echo "❌ Error: Failed to add network connection."
  exit 1
fi

echo "🔒 Setting up WPA2-Personal Security and Sharing mode..."
# Disable PMF (Protected Management Frames) and Wi-Fi Power Save to prevent "802.1X supplicant took too long to authenticate" timeouts on Raspberry Pi
nmcli connection modify "$CON_NAME" \
  802-11-wireless.mode ap \
  802-11-wireless.band bg \
  802-11-wireless.powersave 2 \
  ipv4.method shared \
  wifi-sec.key-mgmt wpa-psk \
  wifi-sec.psk "$PASSWORD" \
  wifi-sec.pmf 1

# 5. Activate the Hotspot
echo "🚀 Starting AuraDeck Local Hotspot..."
nmcli connection up "$CON_NAME"

if [ $? -eq 0 ]; then
  echo "=========================================================="
  echo "✅ SUCCESS: AuraDeck Hotspot is now ACTIVE!"
  echo "----------------------------------------------------------"
  echo "  📶 Wi-Fi SSID  : $SSID"
  echo "  🔑 Password    : $PASSWORD"
  echo "  🔌 Gateway IP  : 10.42.0.1 (This is your MQTT & API Broker Server IP)"
  echo "----------------------------------------------------------"
  echo "  ESP32-S3 จะสามารถเชื่อมต่อ Wi-Fi และคุยกับ MQTT ได้ทันทีที่เปิดเครื่อง"
  echo "=========================================================="
else
  echo "❌ Error: Failed to start the hotspot."
  exit 1
fi
