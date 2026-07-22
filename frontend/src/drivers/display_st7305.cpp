/**
 * @file display_st7305.cpp
 * @brief High-performance custom SPI display driver for the ST7305 controller.
 */

#include "display_st7305.h"
#include "config.h"

// SPI transaction settings (10MHz is extremely safe and fast on the ST7305)
static const SPISettings ST7305_SPI_SETTINGS(10000000, MSBFIRST, SPI_MODE0);

ST7305Display::ST7305Display() {}

ST7305Display::~ST7305Display() {
    if (m_buffer) free(m_buffer);
    if (m_pixel_index_lut) free(m_pixel_index_lut);
    if (m_pixel_bit_lut) free(m_pixel_bit_lut);
}

bool ST7305Display::begin() {
    // 1. Allocate Framebuffer
#if defined(BOARD_HAS_PSRAM)
    m_buffer = (uint8_t*)ps_malloc(BUFFER_SIZE);
#else
    m_buffer = (uint8_t*)malloc(BUFFER_SIZE);
#endif

    if (!m_buffer) {
        Serial.println("❌ Error: Failed to allocate ST7305 framebuffer!");
        return false;
    }
    memset(m_buffer, 0xFF, BUFFER_SIZE); // Default to white (0xFF)

    // 2. Precompute Coordinate LUT
    precomputeLUT();

    // 3. Configure Control Pins
    pinMode(PIN_LCD_CS, OUTPUT);
    pinMode(PIN_LCD_RS, OUTPUT);
    pinMode(PIN_LCD_RESET, OUTPUT);
    
    digitalWrite(PIN_LCD_CS, HIGH);
    digitalWrite(PIN_LCD_RS, HIGH);

    // 4. Initialize Hardware SPI
    SPI.begin(PIN_LCD_SCL, -1, PIN_LCD_SDA, -1);

    // 5. Hardware Reset Sequence
    hardwareReset();

    // 6. Send Driver Initialization Settings
    initDisplayRegisters();

    Serial.println("✅ ST7305 RLCD Display Driver initialized successfully.");
    return true;
}

void ST7305Display::precomputeLUT() {
    uint32_t total_pixels = (uint32_t)WIDTH * HEIGHT;
    
    // Try to allocate LUTs in PSRAM to prevent heap fragmentation
#if defined(BOARD_HAS_PSRAM)
    m_pixel_index_lut = (uint16_t*)ps_malloc(total_pixels * sizeof(uint16_t));
    m_pixel_bit_lut = (uint8_t*)ps_malloc(total_pixels * sizeof(uint8_t));
#else
    m_pixel_index_lut = (uint16_t*)malloc(total_pixels * sizeof(uint16_t));
    m_pixel_bit_lut = (uint8_t*)malloc(total_pixels * sizeof(uint8_t));
#endif

    if (m_pixel_index_lut && m_pixel_bit_lut) {
        m_use_lut = true;
        
        const uint16_t H4 = HEIGHT >> 2; // Vertical blocks (300 / 4 = 75)

        for (uint16_t y = 0; y < HEIGHT; y++) {
            const uint16_t inv_y = HEIGHT - 1 - y;
            const uint16_t block_y = inv_y >> 2;
            const uint8_t local_y = inv_y & 3;

            for (uint16_t x = 0; x < WIDTH; x++) {
                const uint16_t byte_x = x >> 1;
                const uint8_t local_x = x & 1;

                const uint32_t buffer_idx = byte_x * H4 + block_y;
                const uint8_t bit = 7 - ((local_y << 1) | local_x);

                const uint32_t pixel_idx = (uint32_t)x * HEIGHT + y;
                m_pixel_index_lut[pixel_idx] = buffer_idx;
                m_pixel_bit_lut[pixel_idx] = (1 << bit);
            }
        }
        Serial.println("🚀 ST7305 precomputed pixel O(1) LUT tables active.");
    } else {
        m_use_lut = false;
        if (m_pixel_index_lut) free(m_pixel_index_lut);
        if (m_pixel_bit_lut) free(m_pixel_bit_lut);
        m_pixel_index_lut = nullptr;
        m_pixel_bit_lut = nullptr;
        Serial.println("⚠️ Warning: PSRAM full. Running display drawing via real-time math fallback.");
    }
}

void ST7305Display::fillScreen(bool is_black) {
    memset(m_buffer, is_black ? 0x00 : 0xFF, BUFFER_SIZE);
}

void ST7305Display::drawPixel(int16_t x, int16_t y, bool is_black) {
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return;

    if (m_use_lut) {
        uint32_t pixel_idx = (uint32_t)x * HEIGHT + y;
        uint16_t buffer_idx = m_pixel_index_lut[pixel_idx];
        uint8_t bit_mask = m_pixel_bit_lut[pixel_idx];

        if (is_black) {
            m_buffer[buffer_idx] &= ~bit_mask;  // Clear bit = black
        } else {
            m_buffer[buffer_idx] |= bit_mask;   // Set bit = white
        }
    } else {
        // Safe on-the-fly math fallback if LUT is inactive
        uint16_t inv_y = HEIGHT - 1 - y;
        uint16_t block_y = inv_y >> 2;
        uint8_t local_y = inv_y & 3;
        uint16_t byte_x = x >> 1;
        uint8_t local_x = x & 1;

        uint32_t buffer_idx = byte_x * 75 + block_y;
        uint8_t bit = 7 - ((local_y << 1) | local_x);

        if (is_black) {
            m_buffer[buffer_idx] &= ~(1 << bit);
        } else {
            m_buffer[buffer_idx] |= (1 << bit);
        }
    }
}

void ST7305Display::flush() {
    if (!m_buffer) return;

    // Ensure display registers are awake and ready for write
    writeCommand(0x38); // High Power Mode
    writeCommand(0x29); // Display On

    // Set column address window (0x12 to 0x2A for 400px width)
    writeCommand(0x2A);
    writeData(0x12);
    writeData(0x2A);

    // Set row address window (0x00 to 0xC7 for 300px height)
    writeCommand(0x2B);
    writeData(0x00);
    writeData(0xC7);

    // Write buffer over SPI
    digitalWrite(PIN_LCD_RS, LOW); // Command Mode
    digitalWrite(PIN_LCD_CS, LOW); // CS Active (LOW)
    SPI.beginTransaction(ST7305_SPI_SETTINGS);
    SPI.transfer(0x2C);            // Ram Write command
    SPI.endTransaction();

    digitalWrite(PIN_LCD_RS, HIGH); // Switch to Data Mode (CS stays LOW)
    SPI.beginTransaction(ST7305_SPI_SETTINGS);
    
    // Transfer entire buffer chunk
    SPI.writeBytes(m_buffer, BUFFER_SIZE);
    
    SPI.endTransaction();
    digitalWrite(PIN_LCD_CS, HIGH); // CS Inactive (HIGH)
}

// Low-level write utilities
void ST7305Display::writeCommand(uint8_t cmd) {
    digitalWrite(PIN_LCD_RS, LOW); // Command Mode
    digitalWrite(PIN_LCD_CS, LOW); // CS Active
    SPI.beginTransaction(ST7305_SPI_SETTINGS);
    SPI.transfer(cmd);
    SPI.endTransaction();
    digitalWrite(PIN_LCD_CS, HIGH); // CS Inactive
}

void ST7305Display::writeData(uint8_t data) {
    digitalWrite(PIN_LCD_RS, HIGH); // Data Mode
    digitalWrite(PIN_LCD_CS, LOW);  // CS Active
    SPI.beginTransaction(ST7305_SPI_SETTINGS);
    SPI.transfer(data);
    SPI.endTransaction();
    digitalWrite(PIN_LCD_CS, HIGH);  // CS Inactive
}

void ST7305Display::hardwareReset() {
    digitalWrite(PIN_LCD_RESET, HIGH);
    delay(50);
    digitalWrite(PIN_LCD_RESET, LOW);
    delay(20);
    digitalWrite(PIN_LCD_RESET, HIGH);
    delay(50);
}

void ST7305Display::initDisplayRegisters() {
    // NVM Load Control
    writeCommand(0xD6);
    writeData(0x17);
    writeData(0x02);

    // Booster Enable (charge pump)
    writeCommand(0xD1);
    writeData(0x01);

    // Gate Voltage Setting (VGH/VGL)
    writeCommand(0xC0);
    writeData(0x11);
    writeData(0x04);

    // VSHP Setting (positive source high power)
    writeCommand(0xC1);
    writeData(0x69);
    writeData(0x69);
    writeData(0x69);
    writeData(0x69);

    // VSLP Setting (positive source low power)
    writeCommand(0xC2);
    writeData(0x19);
    writeData(0x19);
    writeData(0x19);
    writeData(0x19);

    // VSHN Setting (negative source high power)
    writeCommand(0xC4);
    writeData(0x4B);
    writeData(0x4B);
    writeData(0x4B);
    writeData(0x4B);

    // VSLN Setting (negative source low power)
    writeCommand(0xC5);
    writeData(0x19);
    writeData(0x19);
    writeData(0x19);
    writeData(0x19);

    // Oscillator frequency control
    writeCommand(0xD8);
    writeData(0x80);
    writeData(0xE9);

    // Frame Rate Control
    writeCommand(0xB2);
    writeData(0x02);

    // Gate EQ Control (High Power Mode)
    writeCommand(0xB3);
    writeData(0xE5);
    writeData(0xF6);
    writeData(0x05);
    writeData(0x46);
    writeData(0x77);
    writeData(0x77);
    writeData(0x77);
    writeData(0x77);
    writeData(0x76);
    writeData(0x45);

    // Gate EQ Control (Low Power Mode)
    writeCommand(0xB4);
    writeData(0x05);
    writeData(0x46);
    writeData(0x77);
    writeData(0x77);
    writeData(0x77);
    writeData(0x77);
    writeData(0x76);
    writeData(0x45);

    // Gate Timing Control
    writeCommand(0x62);
    writeData(0x32);
    writeData(0x03);
    writeData(0x1F);

    // Source EQ Enable
    writeCommand(0xB7);
    writeData(0x13);

    // Gate Line Setting (100 * 3 = 300 lines)
    writeCommand(0xB0);
    writeData(0x64);

    // Sleep Out
    writeCommand(0x11);
    delay(200); // Required datasheet delay before exit sleep

    // Source Voltage Select
    writeCommand(0xC9);
    writeData(0x00);

    // Memory Data Access Control (MADCTL) - MX=1, DO=1
    writeCommand(0x36);
    writeData(0x48);

    // Data Format Select (1-bit monochrome)
    writeCommand(0x3A);
    writeData(0x11);

    // Gamma Mode (Monochrome)
    writeCommand(0xB9);
    writeData(0x20);

    // Panel Setting
    writeCommand(0xB8);
    writeData(0x29);

    // Display Inversion On
    writeCommand(0x21);

    // Set Column Address Set
    writeCommand(0x2A);
    writeData(0x12);
    writeData(0x2A);

    // Set Row Address Set
    writeCommand(0x2B);
    writeData(0x00);
    writeData(0xC7);

    // Tearing Effect Line On
    writeCommand(0x35);
    writeData(0x00);

    // Auto Power Down Control
    writeCommand(0xD0);
    writeData(0xFF);

    // High Power Mode On
    writeCommand(0x38);

    // Display On
    writeCommand(0x29);
}

void ST7305Display::sleep() {
    writeCommand(0x10); // Sleep In
    Serial.println("💤 ST7305 display entered low-power sleep.");
}

void ST7305Display::wake() {
    writeCommand(0x11); // Sleep Out
    delay(120);         // Wait for registers to power up
    Serial.println("☀️ ST7305 display woke up from sleep.");
}

void ST7305Display::setLowPowerMode(bool enable) {
    if (enable) {
        writeCommand(0x39); // Low Power mode on
        Serial.println("🔋 ST7305 running in Low Power voltage mode.");
    } else {
        writeCommand(0x38); // High Power mode on
        Serial.println("⚡ ST7305 running in High Power voltage mode.");
    }
}
