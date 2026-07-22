#!/bin/bash

# ==============================================================================
# AuraDeck Server-Side Control & Launcher Script
# Script สำหรับเริ่มการทำงานของระบบหลังบ้าน (MQTT Broker + FastAPI Container)
# ==============================================================================

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

echo "=========================================================="
echo "🚀 Starting AuraDeck Backend Stack..."
echo "=========================================================="

# 1. Check for .env file
if [ ! -f "$PROJECT_ROOT/backend/.env" ]; then
  echo "⚠️ Warning: .env file not found in backend/ directory!"
  if [ -f "$PROJECT_ROOT/backend/.env.example" ]; then
    echo "📋 Copying .env.example to .env..."
    cp "$PROJECT_ROOT/backend/.env.example" "$PROJECT_ROOT/backend/.env"
    echo "💡 Please edit $PROJECT_ROOT/backend/.env to set your specific API tokens."
  else
    echo "❌ Error: Neither .env nor .env.example exists."
    exit 1
  fi
fi

# 2. Check if Docker is installed
if ! command -v docker &> /dev/null; then
  echo "❌ Error: Docker is not installed. Please install Docker first."
  exit 1
fi

# 3. Start containers using Docker Compose
echo "🐳 Launching Docker containers (MQTT + Backend API)..."
cd "$PROJECT_ROOT"

# Check if 'docker compose' is supported, otherwise fallback to 'docker-compose'
if docker compose version &> /dev/null; then
  DOCKER_CMD="docker compose"
elif docker-compose version &> /dev/null; then
  DOCKER_CMD="docker-compose"
else
  echo "❌ Error: Neither 'docker compose' nor 'docker-compose' commands are available."
  exit 1
fi

echo "⚙️ Executing: $DOCKER_CMD up -d --build"
$DOCKER_CMD up -d --build

if [ $? -eq 0 ]; then
  echo "=========================================================="
  echo "✅ SUCCESS: AuraDeck Server is now RUNNING!"
  echo "----------------------------------------------------------"
  echo "  🌐 API Control Center : http://localhost:8000"
  echo "  🔌 MQTT Broker        : Port 1883"
  echo "----------------------------------------------------------"
  echo "  ใช้คำสั่ง: docker compose logs -f backend  เพื่อดู log การทำงาน"
  echo "  ใช้คำสั่ง: docker compose down             เพื่อปิดการทำงาน"
  echo "=========================================================="
else
  echo "❌ Error: Failed to launch Docker containers."
  exit 1
fi
