# ESP32-S3 AuraDeck: Smart Ambient RLCD Dashboard

An ambient, ultra-high-contrast, low-power desktop information display utilizing a **Waveshare ESP32-S3-RLCD-4.2** reflective monochrome screen. 

The system leverages a **Raspberry Pi** (or any local home server running Docker) to manage heavy integration logic, OAuth2 credential lifecycles, and backend pollers. It passes simplified, lightweight JSON payloads to the ESP32 via REST and MQTT.

---

## 🏗️ System Architecture

```
                 [ Third-Party Cloud APIs ]
 (Google Calendar / Tasks, Spotify, Yahoo Finance, GCP Billing)
                            │
                            ▼
          ┌───────────────────────────────────┐
          │      Raspberry Pi (Backend)       │
          │  - Python FastAPI Service         │
          │  - Eclipse Mosquitto Broker       │
          │  - Periodic Cron Background Poll  │
          │  - OAuth2 Silent Refresh Manager  │
          └─────────────────┬─────────────────┘
                            │ Wi-Fi (JSON Payload)
                            ▼
          ┌───────────────────────────────────┐
          │  ESP32-S3 RLCD Smart Terminal     │
          │  - 4.2" 1-bit reflective LCD      │
          │  - LVGL v8 Graphics Engine        │
          │  - Real-time Temp/Hum (SHTC3)     │
          │  - Hardware RTC (PCF85063)        │
          └───────────────────────────────────┘
```

---

## 🛠️ Hardware Requirements & Specifications
*   **Target Board:** Waveshare ESP32-S3-RLCD-4.2 (ESP32-S3-WROOM-1-N16R8 with 16MB Flash and 8MB Octal PSRAM)
*   **Display:** 4.2" Reflective LCD (ST7305 Driver, SPI 4-wire, 400x300 Resolution, Landscape orientation)
*   **Sensors:** SHTC3 Temperature & Humidity sensor (I2C)
*   **RTC:** PCF85063 Real-Time Clock with battery backup (I2C)
*   **Button:** Onboard USER key for hardware-debounced page cycling (GPIO18)

---

## 📖 Comprehensive Guides & Setup Manuals

To make the codebase highly modular and maintainable, setup details, firmware flashing guides, and hardware-specific graphics explanations have been organized into the following manuals within the `docs/` directory:

### ⚡ [Backend Server Setup & Developer Guide](./docs/backend.md)
*   **Docker Ingestion & Deployment**: Step-by-step procedures to build and start the cluster (`FastAPI` & `Mosquitto Broker`) in the background.
*   **Raspberry Pi Local Access Point**: Restructuring the Pi into a standalone Wi-Fi hotspot, and solving supplicant or Broadcom driver handshake timeouts.
*   **Automatic Startup Configuration**: Configuring Docker daemon, systemd service unit, or cron `@reboot` to autostart backend on Pi restart.
*   **Secure OAuth via ngrok & Token Mirroring**: Tunneling redirects safely and managing fallback tokens locally.
*   **Developer Sandbox**: Publishing mock JSON payloads to test screen layouts instantaneously without live cloud logins.
*   **System Diagnostics**: Log checking and common Linux/Docker privilege troubleshooting.

### 🔌 [Frontend Firmware & Graphics Guide](./docs/frontend.md)
*   **PlatformIO IDE Compiler Environment**: Setup instructions for ESP32-S3 firmware compiling and automatic dependency installations.
*   **Physical Pinout Configurations**: Wiring schematics for ST7305 RLCD screen SPI connections, I2C ambient sensors, and user interrupt buttons.
*   **Interactive Navigation Panels**: Breakdown of screen displays (Pages 0–6) cycling with a physical debounced button.
*   **Graphics Engine (LVGL v8 Monochrome)**: Meticulous rendering optimizations for reflective displays, custom PSRAM allocations, thread-safe page switching, and Thai vowel stacking (C++ ThaiReshaper).
*   **Network Resilience**: Bulletproof I2C bus self-healing, automatic clock synchronization, and graceful error degradations.

---

## 🚀 ลำดับขั้นตอนการเริ่มต้นใช้งานระบบ (End-to-End System Startup)

เมื่อเริ่มติดตั้งระบบครั้งแรก หรือต้องการเปิดใช้งานใหม่ ให้ทำตามลำดับขั้นตอนดังนี้:

```
┌─────────────────────────┐     ┌─────────────────────────┐     ┌─────────────────────────┐
│   1. Raspberry Pi       │ ──► │   2. OAuth Login        │ ──► │   3. ESP32-S3 Display   │
│ - Docker Compose (Up)   │     │ - รัน ngrok tunnel      │     │ - เปิดเครื่อง เชื่อม Wi-Fi│
│ - Tailscale (รีโมต/SSH) │     │ - ล็อกอิน Google/Spotify│     │ - กรอก PIN 6 หลัก Pairing │
│ - Wi-Fi AP Hotspot      │     │ - ปิด ngrok ได้เลย      │     │ - ดึงข้อมูลอัตโนมัติ      │
└─────────────────────────┘     └─────────────────────────┘     └─────────────────────────┘
```

### ขั้นที่ 1: เตรียม Raspberry Pi (Server)
1. **ติดตั้งและรัน Tailscale** (สำหรับ SSH / จัดการระบบข้ามวงแลน):
   ```bash
   sudo tailscale up
   ```
2. **สร้าง Wi-Fi Hotspot** สำหรับให้บอร์ด ESP32 เกาะ (IP: `10.42.0.1`):
   ```bash
   sudo ./backend/scripts/setup_ap.sh
   ```
3. **เปิดระบบ Backend & MQTT Cluster**:
   ```bash
   ./backend/scripts/start_backend.sh
   ```
   *(ระบบรันผ่าน Docker พร้อมตั้ง `restart: unless-stopped` เพื่อเปิดอัตโนมัติเมื่อเปิดเครื่อง)*

### ขั้นที่ 2: ผูกบัญชี Cloud Services (Google & Spotify)
เนื่องจาก Google และ Spotify บังคับให้ใช้ HTTPS ในการยืนยันตัวตน:
1. **เปิด ngrok HTTPS Tunnel บน Pi**:
   ```bash
   ngrok http 8000
   ```
2. **เข้าไปที่ URL `https://xxxx.ngrok-free.app`** จากมือถือหรือคอมพิวเตอร์
3. **กด "Sign In with Google" และเชื่อมต่อ "Spotify"** ให้เรียบร้อย
4. เมื่อโทเค็นถูกบันทึกลงในระบบแล้ว สามารถกด `Ctrl + C` เพื่อ **ปิด ngrok ได้ทันที** (ระบบจะ Auto Refresh Token ในเบื้องหลังผ่าน Wi-Fi ต่อเนื่อง)

### ขั้นที่ 3: เปิดหน้าจอ ESP32-S3 Display & Pairing
1. เสียบสายไฟ USB หรือเปิดสวิตช์แบตเตอรี่ของบอร์ด ESP32-S3
2. ตัวบอร์ดจะเชื่อมต่อเข้ากับ Hotspot `AuraDeck_AP` อัตโนมัติ
3. หน้าจอจะแสดง **PIN 6 หลัก (TV-Style Pairing)**
4. เปิดหน้าเว็บ Dashboard (`http://10.42.0.1:8000` หรือผ่าน Tailscale `http://kea-pi-server:8000`) ไปที่เมนู **Device Pairing** แล้วกรอก PIN เพื่อผูกหน้าจอ
5. หน้าจอจะเข้าสู่หน้า Dashboard หลัก และอัปเดตข้อมูลอัตโนมัติทันที

---

### ☀️ การใช้งานในชีวิตประจำวัน (Daily Operation)
* **ไม่ต้องรันคำสั่งใดๆ ซ้ำ**: เมื่อเปิด Raspberry Pi ขึ้นมาระบบ Docker และ Tailscale จะเริ่มทำงานให้อัตโนมัติ
* **หน้าจอ ESP32**: เมื่อจ่ายไฟจะบูตและดึงข้อมูลทันที แสดงสถานะแบตเตอรี่, เวลา, สภาพอากาศ, และ Widget ต่างๆ ตลอดเวลา

---

## 🌐 การตั้งค่ารีโมตเข้า Raspberry Pi ด้วย Tailscale (Mesh VPN)

### ทำไมต้อง Tailscale?

ในเครือข่ายระดับองค์กร (Enterprise / Domain Network) มักมีการแยกวง **Wi-Fi** กับ **LAN** ออกจากกัน (คนละ Subnet/VLAN) รวมถึงบล็อก Multicast Traffic ทำให้:

- ❌ ใช้ `mDNS` (ชื่อ `.local`) ค้นหาเครื่องข้ามวงไม่ได้
- ❌ Fix IP ท้องถิ่นไม่เสถียร — เปลี่ยน VLAN หรือย้ายห้องก็ต้องตั้งค่าใหม่
- ❌ เข้าถึง Raspberry Pi จากภายนอกออฟฟิศ (Remote / WFH) ไม่ได้เลย

**[Tailscale](https://tailscale.com/)** เป็น Mesh VPN บนพื้นฐาน **WireGuard** ที่ช่วยให้เครื่อง Client (คอมพิวเตอร์) และ Server (Raspberry Pi) เชื่อมต่อกันได้เสมือนอยู่ในวง LAN เดียวกันตลอดเวลา:

- ✅ ได้รับ **IP คงที่** (`100.x.y.z`) ไม่ว่าจะต่อผ่าน Wi-Fi, LAN หรือ Hotspot มือถือ
- ✅ รองรับ **MagicDNS** — เรียกเครื่องด้วยชื่อ hostname ได้โดยตรง (เช่น `kea-pi-server`)
- ✅ เข้ารหัส End-to-End ด้วย WireGuard — ปลอดภัยแม้ผ่านเครือข่ายสาธารณะ
- ✅ ทำงานจากที่ไหนก็ได้ — บ้าน, คาเฟ่, ต่างประเทศ

---

### ขั้นตอนที่ 1: ติดตั้งบน Raspberry Pi (Server)

#### 1.1 ติดตั้ง Tailscale

```bash
curl -fsSL https://tailscale.com/install.sh | sh
```

#### 1.2 เริ่มต้นและเชื่อมต่อบัญชี

สามารถเลือกวิธีเชื่อมต่อได้ 2 รูปแบบ:

##### 🔑 วิธีที่ 1: ใช้ Auth Key (แนะนำ — สะดวกและไม่ต้องคลิกลิงก์)
1. ไปที่ **[Tailscale Admin Settings > Keys](https://login.tailscale.com/admin/settings/keys)** บนเครื่องคอมพิวเตอร์
2. กด **Generate auth key...** แล้วคัดลอก Key (`tskey-auth-...`)
3. รันคำสั่งบน Raspberry Pi:
   ```bash
   sudo tailscale up --authkey="tskey-auth-xxxxxxxxxxxx"
   ```
   *(ระบบจะยืนยันตัวตนสำเร็จทันที ไม่ต้องเปิดเว็บ)*

##### 🌐 วิธีที่ 2: ล็อกอินผ่านลิงก์เบราว์เซอร์
```bash
sudo tailscale up
```
คำสั่งนี้จะแสดง **URL สำหรับยืนยันตัวตน** ในเทอร์มินัล เช่น:
```
To authenticate, visit:
    https://login.tailscale.com/a/xxxxxxxxxxxx
```
ให้คัดลอกลิงก์ไปเปิดในเบราว์เซอร์แล้วล็อกอินด้วย Google / Microsoft / GitHub

> [!WARNING]
> **ถ้าเจอปัญหา `403 Session Expired` ตอนล็อกอินด้วย Google:**
> - **สาเหตุ:** ลิงก์ยืนยันตัวตนหมดอายุอย่างรวดเร็ว หรือมีปัญหา Session ค้างในเบราว์เซอร์
> - **วิธีแก้:** 
>   1. **ใช้ Auth Key (วิธีที่ 1 ด้านบน)** — ชัวร์ที่สุด 100%
>   2. หรือรัน `sudo tailscale up --reset` แล้วนำลิงก์ใหม่ไปเปิดใน **หน้าต่างไม่ระบุตัวตน (Incognito Window)** ทันที
>   3. ตรวจสอบเวลาเครื่อง Pi ให้ตรง: `sudo timedatectl set-ntp true`

#### 1.3 ตรวจสอบ IP ของ Tailscale

```bash
# ดู Tailscale IP (IPv4)
tailscale ip -4

# ดูสถานะการเชื่อมต่อทั้งหมด
tailscale status
```

ตัวอย่างผลลัพธ์:
```
100.64.0.5    kea-pi-server    user@gmail.com    linux    -
100.64.0.2    dev-laptop       user@gmail.com    windows  -
```

---

### ขั้นตอนที่ 2: ตั้งค่าฝั่ง Client (เครื่อง Developer)

#### 2.1 ดาวน์โหลดและติดตั้ง

| ระบบปฏิบัติการ | วิธีติดตั้ง |
|----------------|-------------|
| **Windows** | ดาวน์โหลดจาก [tailscale.com/download](https://tailscale.com/download) หรือ Microsoft Store |
| **macOS** | ดาวน์โหลดจาก [Mac App Store](https://apps.apple.com/app/tailscale/id1475387142) หรือ `brew install tailscale` |
| **Linux** | `curl -fsSL https://tailscale.com/install.sh \| sh && sudo tailscale up` |

#### 2.2 ล็อกอินด้วยบัญชีเดียวกัน

เปิดแอป Tailscale บน Client แล้วล็อกอินด้วย **บัญชีเดียวกัน** กับที่ใช้บน Raspberry Pi (หรืออยู่ใน Tailnet เดียวกัน)

#### 2.3 ยืนยันว่าเห็น Raspberry Pi

ตรวจสอบผ่าน **Tailscale Admin Console** ([login.tailscale.com/admin/machines](https://login.tailscale.com/admin/machines)) หรือใน Client App:

- ✅ Node ของ Raspberry Pi (เช่น `kea-pi-server`) ขึ้นสถานะ **Connected**
- ✅ แสดง IP `100.x.y.z` ที่ถูกต้อง

---

### ขั้นตอนที่ 3: เชื่อมต่อ SSH & Development

#### 3.1 SSH ผ่าน Tailscale IP

```bash
ssh pi@100.64.0.5
```

#### 3.2 SSH ผ่าน MagicDNS (แนะนำ)

Tailscale รองรับ **MagicDNS** ซึ่งแปลง hostname เป็น Tailscale IP อัตโนมัติ ไม่ต้องจำ IP:

```bash
ssh pi@kea-pi-server
```

#### 3.3 ตั้งค่า SSH Config สำหรับเชื่อมต่อแบบสั้น (แนะนำ)

สร้างหรือแก้ไขไฟล์ `~/.ssh/config` บนเครื่อง Client:

```ssh-config
Host kea-pi
    HostName kea-pi-server          # MagicDNS hostname (หรือใส่ 100.64.0.5)
    User pi
    Port 22
    ServerAliveInterval 60          # ป้องกัน SSH timeout ทุก 60 วินาที
    ServerAliveCountMax 3
```

หลังจากนั้นเชื่อมต่อได้ง่ายๆ ด้วย:

```bash
ssh kea-pi
```

> [!TIP]
> ถ้าใช้ **VS Code Remote-SSH** ให้เพิ่ม Config เดียวกันนี้ แล้วเลือก `kea-pi` จาก Remote Explorer ได้เลย — ทำงานเหมือนเปิดโปรเจคบนเครื่องตัวเอง

---

### ขั้นตอนที่ 4: ทดสอบการเชื่อมต่อ Tailscale

หลังจากติดตั้งทั้งสองฝั่งแล้ว ให้ทดสอบตามลำดับต่อไปนี้:

#### 4.1 Ping Test — ตรวจสอบว่าเครือข่าย Tailscale ทำงาน

```bash
# จากเครื่อง Client → Raspberry Pi
ping kea-pi-server          # ผ่าน MagicDNS
ping 100.64.0.5             # ผ่าน Tailscale IP โดยตรง
```

ผลลัพธ์ที่คาดหวัง:
```
PING kea-pi-server (100.64.0.5): 56 data bytes
64 bytes from 100.64.0.5: icmp_seq=0 ttl=64 time=3.42 ms
```

> [!WARNING]
> ถ้า ping ไม่ผ่าน ให้ตรวจสอบ:
> - ทั้งสองเครื่องล็อกอิน Tailscale ด้วย **บัญชีเดียวกัน** (หรืออยู่ใน Tailnet เดียวกัน)
> - Tailscale daemon ทำงานอยู่: `tailscale status` ต้องแสดง node ทั้งสองฝั่ง
> - Firewall ไม่ได้บล็อก UDP port 41641 (WireGuard)

#### 4.2 SSH Test — ตรวจสอบว่า Remote Shell ใช้งานได้

```bash
ssh pi@kea-pi-server
```

เมื่อเชื่อมต่อสำเร็จ จะเห็น shell prompt ของ Raspberry Pi:
```
pi@kea-pi-server:~ $
```

#### 4.3 Port Connectivity Test — ตรวจสอบว่า Backend Services เข้าถึงได้

```bash
# ทดสอบ FastAPI Backend (Port 8000)
curl http://kea-pi-server:8000/health

# ทดสอบ MQTT Broker (Port 1883)
nc -zv kea-pi-server 1883
```

ผลลัพธ์ที่คาดหวัง:
```
# FastAPI
{"status": "ok"}

# MQTT
Connection to kea-pi-server port 1883 [tcp/*] succeeded!
```

#### 4.4 Tailscale Status — ตรวจสอบสถานะ Node ทั้งหมด

```bash
tailscale status
```

ตัวอย่างผลลัพธ์ที่ถูกต้อง:
```
100.64.0.5    kea-pi-server    user@gmail.com    linux    active; direct 192.168.1.50:41641
100.64.0.2    dev-laptop       user@gmail.com    windows  active; direct 192.168.1.100:41641
```

- **`active; direct`** = เชื่อมต่อแบบ peer-to-peer โดยตรง (เร็วที่สุด)
- **`active; relay`** = เชื่อมต่อผ่าน DERP relay (ยังใช้งานได้ แต่ช้ากว่า — มักเกิดเมื่อ NAT เข้มงวด)

> [!TIP]
> ใช้ `tailscale ping kea-pi-server` เพื่อดู latency จริงและตรวจสอบว่าเป็น direct connection หรือผ่าน relay:
> ```bash
> tailscale ping kea-pi-server
> # pong from kea-pi-server (100.64.0.5) via 192.168.1.50:41641 in 3ms
> ```

---

## 🌀 Architectural Features in AuraDeck 2.0

*   **Multi-Profile Session Sandboxing**: Users can register independent environments, Service Account keys, and custom Google/Spotify credentials.
*   **Multi-List Google Tasks Aggregation**: Automatically merges and prefixes tasks from multiple user-checked lists in parallel.
*   **TV-Style PIN Screen Pairing**: Registers physical screen MAC addresses wirelessly with standard 6-digit verification codes.
*   **D-Bus Container Access Point Toggles**: FastAPI containers interact with host NetworkManager services securely without root privilege elevations.
*   **High-End Dark UI**: Modern glassmorphic web dashboard providing live AP statuses, device pairings, and credentials configuration.
