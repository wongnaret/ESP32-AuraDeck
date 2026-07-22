/**
 * @file thai_reshaper.h
 * @brief Helper class to handle Thai Unicode reshaping and prevent floating vowels (สระลอย).
 */

#pragma once

#include <Arduino.h>

class ThaiReshaper {
public:
    /**
     * @brief Reshapes an input Thai UTF-8 string to resolve stacking vowels and tone marks.
     * Re-orders character bytes so they render beautifully without "สระลอย" overlays.
     * @param input Raw UTF-8 string.
     * @return Reshaped UTF-8 string.
     */
    static String reshape(const String& input);
    static String reshape(const char* input);

private:
    // Helper UTF-8 decoding and identification utilities
    static bool isThaiConsonant(uint16_t unicode);
    static bool isThaiUpperVowel(uint16_t unicode);
    static bool isThaiToneMark(uint16_t unicode);
    static bool isThaiLowerVowel(uint16_t unicode);
    
    static uint32_t decodeUTF8(const char* str, int& index);
    static String encodeUTF8(uint32_t unicode);
};
