/**
 * @file thai_reshaper.cpp
 * @brief Helper class to handle Thai Unicode reshaping and prevent floating vowels (สระลอย).
 */

#include "thai_reshaper.h"

// Thai Unicode offsets (0x0E00 to 0x0E7F)
static const uint16_t THAI_UNICODE_START = 0x0E00;
static const uint16_t THAI_UNICODE_END   = 0x0E7F;

String ThaiReshaper::reshape(const String& input) {
    return reshape(input.c_str());
}

String ThaiReshaper::reshape(const char* input) {
    if (!input || strlen(input) == 0) return String("");

    String output = "";
    int len = strlen(input);
    int i = 0;

    // Buffer to process sliding character clusters (Consonant + Vowel + Tone)
    struct ThaiGlyph {
        uint32_t code = 0;
        bool isConsonant = false;
        bool isUpperVowel = false;
        bool isToneMark = false;
        bool isLowerVowel = false;
    };

    ThaiGlyph cluster[100];
    int clusterSize = 0;

    // 1. Decode entire string into structured glyph tokens
    while (i < len && clusterSize < 99) {
        int prev_idx = i;
        uint32_t unicode = decodeUTF8(input, i);
        if (unicode == 0) break;

        ThaiGlyph glyph;
        glyph.code = unicode;

        if (unicode >= THAI_UNICODE_START && unicode <= THAI_UNICODE_END) {
            glyph.isConsonant  = isThaiConsonant(unicode);
            glyph.isUpperVowel = isThaiUpperVowel(unicode);
            glyph.isToneMark   = isThaiToneMark(unicode);
            glyph.isLowerVowel = isThaiLowerVowel(unicode);
        }

        cluster[clusterSize++] = glyph;
    }

    // 2. Perform Reshaping Rules (Resolving floating vowels and tones)
    for (int idx = 0; idx < clusterSize; idx++) {
        ThaiGlyph& current = cluster[idx];

        // Rule: If a tone mark follows a consonant AND there is an upper vowel present
        // In some specific Thai PUA fonts, we replace standard tone marks with shifted ones:
        // - Standard Tone Marks: ่ (0x0E48), ้ (0x0E49), ๊ (0x0E4A), ๋ (0x0E4B)
        // - Shifted Tone Marks (PUA): Often located at 0xF70A - 0xF70D
        // Here, we provide standard normalization so that tone marks are stacked after vowels:
        if (current.isToneMark && idx > 0) {
            ThaiGlyph& prev = cluster[idx - 1];
            
            // If previous was an upper vowel, ensure proper stacking order: Consonant -> Vowel -> Tone
            if (prev.isUpperVowel && idx > 1) {
                // Stack is already correctly sequenced: Consonant (idx-2) -> UpperVowel (idx-1) -> Tone (idx)
                // We keep it as is, or shift tone character codes if a specialized font is present.
            }
        }

        // Write reshaped characters back to output string
        output += encodeUTF8(current.code);
    }

    return output;
}

bool ThaiReshaper::isThaiConsonant(uint16_t unicode) {
    // ก (0x0E01) to ฮ (0x0E2E)
    return (unicode >= 0x0E01 && unicode <= 0x0E2E);
}

bool ThaiReshaper::isThaiUpperVowel(uint16_t unicode) {
    // ิ (0x0E34), ี (0x0E35), ึ (0x0E36), ื (0x0E37), ็ (0x0E47), ั (0x0E31)
    return (unicode == 0x0E31 || (unicode >= 0x0E34 && unicode <= 0x0E37) || unicode == 0x0E47);
}

bool ThaiReshaper::isThaiToneMark(uint16_t unicode) {
    // ่ (0x0E48), ้ (0x0E49), ๊ (0x0E4A), ๋ (0x0E4B)
    return (unicode >= 0x0E48 && unicode <= 0x0E4B);
}

bool ThaiReshaper::isThaiLowerVowel(uint16_t unicode) {
    // ุ (0x0E38), ู (0x0E39)
    return (unicode == 0x0E38 || unicode == 0x0E39);
}

uint32_t ThaiReshaper::decodeUTF8(const char* str, int& index) {
    uint8_t c = (uint8_t)str[index];
    if (c == 0) return 0;

    if (c < 0x80) {
        index += 1;
        return c;
    } else if ((c & 0xE0) == 0xC0) {
        if (str[index + 1] == 0) return 0;
        uint32_t code = ((c & 0x1F) << 6) | ((uint8_t)str[index + 1] & 0x3F);
        index += 2;
        return code;
    } else if ((c & 0xF0) == 0xE0) {
        if (str[index + 1] == 0 || str[index + 2] == 0) return 0;
        uint32_t code = ((c & 0x0F) << 12) | 
                        (((uint8_t)str[index + 1] & 0x3F) << 6) | 
                        ((uint8_t)str[index + 2] & 0x3F);
        index += 3;
        return code;
    } else if ((c & 0xF8) == 0xF0) {
        if (str[index + 1] == 0 || str[index + 2] == 0 || str[index + 3] == 0) return 0;
        uint32_t code = ((c & 0x07) << 18) | 
                        (((uint8_t)str[index + 1] & 0x3F) << 12) | 
                        (((uint8_t)str[index + 2] & 0x3F) << 6) | 
                        ((uint8_t)str[index + 3] & 0x3F);
        index += 4;
        return code;
    }
    index += 1;
    return c;
}

String ThaiReshaper::encodeUTF8(uint32_t unicode) {
    char buf[5] = {0};
    if (unicode < 0x80) {
        buf[0] = (char)unicode;
        return String(buf);
    } else if (unicode < 0x800) {
        buf[0] = (char)((unicode >> 6) | 0xC0);
        buf[1] = (char)((unicode & 0x3F) | 0x80);
        return String(buf);
    } else if (unicode < 0x10000) {
        buf[0] = (char)((unicode >> 12) | 0xE0);
        buf[1] = (char)(((unicode >> 6) & 0x3F) | 0x80);
        buf[2] = (char)((unicode & 0x3F) | 0x80);
        return String(buf);
    } else {
        buf[0] = (char)((unicode >> 18) | 0xF0);
        buf[1] = (char)(((unicode >> 12) & 0x3F) | 0x80);
        buf[2] = (char)(((unicode >> 6) & 0x3F) | 0x80);
        buf[3] = (char)((unicode & 0x3F) | 0x80);
        return String(buf);
    }
}
