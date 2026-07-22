/**
 * @file display_st7305.h
 * @brief High-performance custom SPI display driver for the ST7305 controller.
 */

#pragma once

#include <Arduino.h>
#include <SPI.h>

class ST7305Display {
public:
    static const uint16_t WIDTH = 400;
    static const uint16_t HEIGHT = 300;
    static const uint32_t BUFFER_SIZE = (WIDTH * HEIGHT) / 8; // 15,000 bytes

    ST7305Display();
    ~ST7305Display();

    /**
     * @brief Initializes the ST7305 display hardware and precomputes lookups.
     * @return true if successful, false otherwise.
     */
    bool begin();

    /**
     * @brief Clear the frame buffer with a specific color.
     * @param is_black true for black, false for white.
     */
    void fillScreen(bool is_black);

    /**
     * @brief Draws a single pixel to the local framebuffer.
     * @param x horizontal coordinate (0-399)
     * @param y vertical coordinate (0-299)
     * @param is_black true for black (active), false for white (inactive).
     */
    void drawPixel(int16_t x, int16_t y, bool is_black);

    /**
     * @brief Flushes the local framebuffer to the physical ST7305 display panel.
     */
    void flush();

    /**
     * @brief Places the display into low-power sleep mode (~10uA).
     */
    void sleep();

    /**
     * @brief Wakes the display up from sleep mode.
     */
    void wake();

    /**
     * @brief Switches the display driver voltages to low-power update modes.
     */
    void setLowPowerMode(bool enable);

    /**
     * @brief Returns a pointer to the raw monochrome framebuffer.
     */
    uint8_t* getBuffer() { return m_buffer; }

private:
    uint8_t* m_buffer = nullptr;
    
    // Fast O(1) coordinate mapping lookup tables
    uint16_t* m_pixel_index_lut = nullptr;
    uint8_t* m_pixel_bit_lut = nullptr;
    bool m_use_lut = false;

    // Helper functions for SPI transactions
    void writeCommand(uint8_t cmd);
    void writeData(uint8_t data);
    void hardwareReset();
    void initDisplayRegisters();
    void precomputeLUT();
};
