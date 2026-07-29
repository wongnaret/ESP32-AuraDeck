/*******************************************************************************
 * Size: 16 px
 * Bpp: 1
 * Opts: --font Prompt-Regular.ttf -r 0x20-0x7E,0xE01-0xE5B --size 16 --bpp 1 --format lvgl --lv-include lvgl.h --output frontend/src/ui/fonts/lv_font_prompt_16.c --lv-font-name lv_font_prompt_16
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl.h"
#endif

#ifndef LV_FONT_PROMPT_16
#define LV_FONT_PROMPT_16 1
#endif

#if LV_FONT_PROMPT_16

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xff, 0xff, 0x3c,

    /* U+0022 "\"" */
    0xff, 0xf0,

    /* U+0023 "#" */
    0x8, 0x41, 0x18, 0x62, 0x3f, 0xf1, 0x18, 0x23,
    0xc, 0x47, 0xfe, 0x23, 0xc, 0x41, 0x88, 0x0,

    /* U+0024 "$" */
    0x18, 0x3c, 0x7e, 0xd8, 0xd8, 0xd8, 0x78, 0x3e,
    0x1f, 0x1b, 0x1b, 0x9b, 0xfc, 0x18, 0x18,

    /* U+0025 "%" */
    0x78, 0xc6, 0x66, 0x21, 0x61, 0x9b, 0x7, 0xb0,
    0x1, 0x78, 0x1e, 0x61, 0xa1, 0xd, 0x8, 0xcc,
    0xc6, 0x3c,

    /* U+0026 "&" */
    0x3c, 0xc, 0xc1, 0x98, 0x17, 0x1, 0x80, 0xf1,
    0xb9, 0x36, 0x16, 0xc1, 0x9c, 0x78, 0xfb, 0x80,

    /* U+0027 "'" */
    0xfc,

    /* U+0028 "(" */
    0x32, 0x66, 0xcc, 0xcc, 0xcc, 0xc6, 0x63, 0x10,

    /* U+0029 ")" */
    0xc4, 0x66, 0x33, 0x33, 0x33, 0x36, 0x6c, 0x80,

    /* U+002A "*" */
    0x25, 0x5c, 0xad, 0x80,

    /* U+002B "+" */
    0x31, 0xbe, 0x63, 0x0,

    /* U+002C "," */
    0x5a, 0x0,

    /* U+002D "-" */
    0xf8,

    /* U+002E "." */
    0xc0,

    /* U+002F "/" */
    0xc, 0x21, 0x86, 0x10, 0xc2, 0x18, 0x61, 0xc,
    0x30,

    /* U+0030 "0" */
    0x3e, 0x31, 0x98, 0xd8, 0x3c, 0x1e, 0xf, 0x7,
    0x83, 0x63, 0x31, 0x8f, 0x80,

    /* U+0031 "1" */
    0x1b, 0xf6, 0x31, 0x8c, 0x63, 0x18, 0xc6,

    /* U+0032 "2" */
    0x7c, 0x46, 0x3, 0x3, 0x3, 0x6, 0xc, 0x18,
    0x30, 0x60, 0xff,

    /* U+0033 "3" */
    0x7c, 0x87, 0x3, 0x3, 0x6, 0x3c, 0x7, 0x3,
    0x3, 0x7, 0xfc,

    /* U+0034 "4" */
    0x6, 0x7, 0x7, 0x82, 0xc2, 0x63, 0x33, 0x19,
    0xff, 0x6, 0x3, 0x1, 0x80,

    /* U+0035 "5" */
    0x7e, 0xc0, 0xc0, 0xc0, 0xfc, 0x6, 0x3, 0x3,
    0x3, 0x86, 0xfc,

    /* U+0036 "6" */
    0x1c, 0x60, 0x60, 0xc0, 0xde, 0xe7, 0xc3, 0xc3,
    0xc3, 0x66, 0x3c,

    /* U+0037 "7" */
    0xff, 0x3, 0x2, 0x6, 0x4, 0xc, 0x8, 0x18,
    0x10, 0x30, 0x30,

    /* U+0038 "8" */
    0x7e, 0xe7, 0xc3, 0xc3, 0x3c, 0x66, 0xc3, 0xc3,
    0xc3, 0x66, 0x3c,

    /* U+0039 "9" */
    0x3c, 0x66, 0xc3, 0xc3, 0xc3, 0xe7, 0x7b, 0x3,
    0x6, 0x6, 0x38,

    /* U+003A ":" */
    0xf0, 0xf,

    /* U+003B ";" */
    0x6c, 0x0, 0x3, 0xd8,

    /* U+003C "<" */
    0x3, 0x1e, 0x78, 0xe0, 0xf0, 0x3c, 0xf, 0x3,

    /* U+003D "=" */
    0xfe, 0x0, 0x7, 0xf0,

    /* U+003E ">" */
    0xc0, 0x70, 0x1c, 0x7, 0xe, 0x38, 0xe0, 0x80,

    /* U+003F "?" */
    0xfc, 0xc, 0x18, 0x31, 0xc7, 0xc, 0x18, 0x0,
    0x60, 0xc0,

    /* U+0040 "@" */
    0xf, 0xc1, 0xc3, 0x98, 0xc, 0xcf, 0xbc, 0xcd,
    0xec, 0x6f, 0x63, 0x7b, 0x1b, 0xcf, 0x73, 0x0,
    0xc, 0x0, 0x1f, 0x80,

    /* U+0041 "A" */
    0xc, 0x3, 0x1, 0xe0, 0x48, 0x33, 0xc, 0xc2,
    0x11, 0xfe, 0x40, 0xb0, 0x3c, 0xc,

    /* U+0042 "B" */
    0xfe, 0x61, 0xb0, 0xd8, 0x6c, 0x37, 0xf3, 0x7,
    0x83, 0xc1, 0xe0, 0xff, 0xc0,

    /* U+0043 "C" */
    0x1f, 0x8c, 0x16, 0x3, 0x0, 0xc0, 0x30, 0xc,
    0x3, 0x0, 0x60, 0xc, 0x11, 0xf8,

    /* U+0044 "D" */
    0xfe, 0x30, 0xcc, 0x1b, 0x3, 0xc0, 0xf0, 0x3c,
    0xf, 0x3, 0xc1, 0xb0, 0xcf, 0xe0,

    /* U+0045 "E" */
    0xff, 0xc0, 0xc0, 0xc0, 0xc0, 0xfc, 0xc0, 0xc0,
    0xc0, 0xc0, 0xff,

    /* U+0046 "F" */
    0xff, 0xc0, 0xc0, 0xc0, 0xc0, 0xfc, 0xc0, 0xc0,
    0xc0, 0xc0, 0xc0,

    /* U+0047 "G" */
    0x1f, 0x8c, 0x16, 0x3, 0x0, 0xc0, 0x30, 0xfc,
    0xf, 0x3, 0x60, 0xcc, 0x31, 0xf8,

    /* U+0048 "H" */
    0xc1, 0xe0, 0xf0, 0x78, 0x3c, 0x1f, 0xff, 0x7,
    0x83, 0xc1, 0xe0, 0xf0, 0x60,

    /* U+0049 "I" */
    0xff, 0xff, 0xfc,

    /* U+004A "J" */
    0x18, 0xc6, 0x31, 0x8c, 0x63, 0x18, 0xfc,

    /* U+004B "K" */
    0xc1, 0xe1, 0xb1, 0x99, 0x8d, 0x87, 0x83, 0x61,
    0x98, 0xc6, 0x61, 0xb0, 0x60,

    /* U+004C "L" */
    0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,
    0xc0, 0xc0, 0xff,

    /* U+004D "M" */
    0xc0, 0x7c, 0x1f, 0x83, 0xf0, 0x7d, 0x17, 0xa2,
    0xf6, 0xde, 0x53, 0xca, 0x78, 0x8f, 0x11, 0x80,

    /* U+004E "N" */
    0xc1, 0xf0, 0xf8, 0x7e, 0x3d, 0x9e, 0x4f, 0x37,
    0x8f, 0xc3, 0xe1, 0xf0, 0x60,

    /* U+004F "O" */
    0x1f, 0x83, 0xc, 0x60, 0x6c, 0x3, 0xc0, 0x3c,
    0x3, 0xc0, 0x3c, 0x3, 0x60, 0x63, 0xc, 0x1f,
    0x80,

    /* U+0050 "P" */
    0xfe, 0xc3, 0xc3, 0xc3, 0xc3, 0xfe, 0xc0, 0xc0,
    0xc0, 0xc0, 0xc0,

    /* U+0051 "Q" */
    0x1f, 0x83, 0xc, 0x60, 0x6c, 0x3, 0xc0, 0x3c,
    0x3, 0xc0, 0x3c, 0x3, 0x60, 0x63, 0xc, 0x1f,
    0xf0,

    /* U+0052 "R" */
    0xfe, 0xc3, 0xc3, 0xc3, 0xc7, 0xfc, 0xcc, 0xc4,
    0xc6, 0xc6, 0xc3,

    /* U+0053 "S" */
    0x3e, 0xe0, 0xc0, 0xc0, 0xf0, 0x3c, 0xf, 0x3,
    0x3, 0x87, 0xfc,

    /* U+0054 "T" */
    0xff, 0x8c, 0x6, 0x3, 0x1, 0x80, 0xc0, 0x60,
    0x30, 0x18, 0xc, 0x6, 0x0,

    /* U+0055 "U" */
    0xc1, 0xe0, 0xf0, 0x78, 0x3c, 0x1e, 0xf, 0x7,
    0x83, 0xc1, 0xb1, 0x8f, 0x80,

    /* U+0056 "V" */
    0xc0, 0xf0, 0x34, 0x19, 0x86, 0x21, 0xc, 0xc3,
    0x30, 0x58, 0x1e, 0x3, 0x0, 0xc0,

    /* U+0057 "W" */
    0xc3, 0xf, 0xc, 0x3c, 0x30, 0xd1, 0xe2, 0x64,
    0x99, 0x92, 0x62, 0xcd, 0xf, 0x3c, 0x38, 0x70,
    0x61, 0x81, 0x86, 0x0,

    /* U+0058 "X" */
    0xc0, 0x98, 0x63, 0x30, 0xd8, 0x1c, 0x3, 0x1,
    0xe0, 0xd8, 0x33, 0x18, 0x6c, 0xc,

    /* U+0059 "Y" */
    0xc0, 0xd8, 0x66, 0x18, 0xcc, 0x1a, 0x7, 0x80,
    0xc0, 0x30, 0xc, 0x3, 0x0, 0xc0,

    /* U+005A "Z" */
    0xff, 0x3, 0x6, 0xc, 0xc, 0x18, 0x30, 0x30,
    0x60, 0xc0, 0xff,

    /* U+005B "[" */
    0xfc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcf,

    /* U+005C "\\" */
    0x83, 0x4, 0x18, 0x60, 0x83, 0x4, 0x18, 0x60,
    0x83,

    /* U+005D "]" */
    0xf3, 0x33, 0x33, 0x33, 0x33, 0x33, 0x3f,

    /* U+005E "^" */
    0x10, 0x70, 0xa3, 0x64, 0x58, 0xc0,

    /* U+005F "_" */
    0xfc,

    /* U+0060 "`" */
    0xd9, 0x0,

    /* U+0061 "a" */
    0x7c, 0x47, 0x3, 0x7f, 0xc3, 0xc3, 0xc7, 0x7f,

    /* U+0062 "b" */
    0xc0, 0x60, 0x30, 0x18, 0xd, 0xe7, 0x1b, 0x7,
    0x83, 0xc1, 0xe0, 0xf8, 0xdb, 0xc0,

    /* U+0063 "c" */
    0x3e, 0xc3, 0x6, 0xc, 0x18, 0x18, 0x1f,

    /* U+0064 "d" */
    0x1, 0x80, 0xc0, 0x60, 0x33, 0xdb, 0x1f, 0x7,
    0x83, 0xc1, 0xe0, 0xd8, 0xe7, 0xb0,

    /* U+0065 "e" */
    0x3e, 0x31, 0xb0, 0x7f, 0xfc, 0x6, 0x1, 0x80,
    0x7e,

    /* U+0066 "f" */
    0x3b, 0x18, 0xcf, 0xb1, 0x8c, 0x63, 0x18, 0xc0,

    /* U+0067 "g" */
    0x3d, 0xb1, 0xf0, 0x78, 0x3c, 0x1e, 0xd, 0x8e,
    0x7b, 0x1, 0x80, 0xc0, 0xc7, 0xc0,

    /* U+0068 "h" */
    0xc0, 0xc0, 0xc0, 0xc0, 0xde, 0xe3, 0xc3, 0xc3,
    0xc3, 0xc3, 0xc3, 0xc3,

    /* U+0069 "i" */
    0xf3, 0xff, 0xfc,

    /* U+006A "j" */
    0x33, 0x3, 0x33, 0x33, 0x33, 0x33, 0x33, 0xe0,

    /* U+006B "k" */
    0xc0, 0xc0, 0xc0, 0xc0, 0xc6, 0xcc, 0xd8, 0xf0,
    0xd8, 0xd8, 0xcc, 0xc6,

    /* U+006C "l" */
    0xff, 0xff, 0xff,

    /* U+006D "m" */
    0xdd, 0xee, 0x63, 0xc6, 0x3c, 0x63, 0xc6, 0x3c,
    0x63, 0xc6, 0x3c, 0x63,

    /* U+006E "n" */
    0xde, 0xe3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3,

    /* U+006F "o" */
    0x3e, 0x31, 0xb0, 0x78, 0x3c, 0x1e, 0xd, 0x8c,
    0x7c,

    /* U+0070 "p" */
    0xde, 0x71, 0xb0, 0x78, 0x3c, 0x1e, 0xf, 0x8d,
    0xbc, 0xc0, 0x60, 0x30, 0x18, 0x0,

    /* U+0071 "q" */
    0x3d, 0xb1, 0xf0, 0x78, 0x3c, 0x1e, 0xd, 0x8e,
    0x7b, 0x1, 0x80, 0xc0, 0x60, 0x30,

    /* U+0072 "r" */
    0xdf, 0x31, 0x8c, 0x63, 0x18,

    /* U+0073 "s" */
    0x7d, 0x83, 0x7, 0xc3, 0xe0, 0xe1, 0xfe,

    /* U+0074 "t" */
    0x63, 0x19, 0xf6, 0x31, 0x8c, 0x63, 0xe,

    /* U+0075 "u" */
    0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc7, 0x7b,

    /* U+0076 "v" */
    0xc2, 0xc6, 0x46, 0x64, 0x2c, 0x38, 0x38, 0x18,

    /* U+0077 "w" */
    0xc4, 0x39, 0xcf, 0x29, 0xa5, 0x27, 0xbc, 0xf3,
    0x8c, 0x61, 0x8c,

    /* U+0078 "x" */
    0xc2, 0x66, 0x3c, 0x18, 0x38, 0x3c, 0x66, 0xc3,

    /* U+0079 "y" */
    0x41, 0xb0, 0x98, 0xc4, 0x43, 0x60, 0xb0, 0x70,
    0x38, 0xc, 0x4, 0x6, 0xe, 0x0,

    /* U+007A "z" */
    0xfc, 0x31, 0x8c, 0x31, 0x8c, 0x3f,

    /* U+007B "{" */
    0x76, 0x66, 0x66, 0xc6, 0x66, 0x66, 0x67,

    /* U+007C "|" */
    0xff, 0xff, 0xff, 0xff,

    /* U+007D "}" */
    0xe6, 0x66, 0x66, 0x36, 0x66, 0x66, 0x6e,

    /* U+007E "~" */
    0xf0, 0x3c,

    /* U+0E01 "ก" */
    0x3e, 0x31, 0x90, 0x6c, 0x36, 0x1b, 0xd, 0x86,
    0xc3, 0x61, 0x80,

    /* U+0E02 "ข" */
    0xe3, 0x63, 0x63, 0xe3, 0xc3, 0xc3, 0xc3, 0x66,
    0x3c,

    /* U+0E03 "ฃ" */
    0xb3, 0xf3, 0x23, 0x63, 0xc3, 0xc3, 0xc3, 0x66,
    0x3c,

    /* U+0E04 "ค" */
    0x3c, 0x66, 0xc3, 0xc3, 0xc3, 0xf3, 0xc3, 0xc3,
    0xc3,

    /* U+0E05 "ฅ" */
    0x7e, 0xdb, 0xc3, 0xc3, 0xc3, 0xf3, 0xc3, 0xc3,
    0xc3,

    /* U+0E06 "ฆ" */
    0xa3, 0xe3, 0x23, 0x63, 0xc3, 0xc3, 0xc3, 0xe3,
    0xde,

    /* U+0E07 "ง" */
    0x1e, 0x61, 0x90, 0x6c, 0x36, 0x19, 0xc, 0x86,
    0x66, 0x3e, 0x0,

    /* U+0E08 "จ" */
    0xfe, 0x41, 0x80, 0x60, 0x36, 0x1b, 0xc, 0x8c,
    0x4e, 0x3c, 0x0,

    /* U+0E09 "ฉ" */
    0xfc, 0x6, 0x3, 0x3, 0xc3, 0xc3, 0xc3, 0xc7,
    0x7b,

    /* U+0E0A "ช" */
    0xe6, 0x64, 0x6e, 0xe3, 0xc3, 0xc3, 0xc3, 0x66,
    0x3c,

    /* U+0E0B "ซ" */
    0xa6, 0xe4, 0x2e, 0x67, 0xc3, 0xc3, 0xc3, 0x66,
    0x3c,

    /* U+0E0C "ฌ" */
    0x3e, 0x6, 0xc6, 0xf, 0x6, 0x1b, 0xc, 0x36,
    0x18, 0x6c, 0x30, 0xd8, 0x61, 0xb0, 0xe3, 0x79,
    0xbc,

    /* U+0E0D "ญ" */
    0x3e, 0x6, 0xc6, 0xf, 0x6, 0x1b, 0xc, 0x36,
    0x18, 0x6c, 0x30, 0xd8, 0x33, 0x30, 0x3f, 0x78,
    0x6, 0x1, 0xc, 0x3, 0xf0,

    /* U+0E0E "ฎ" */
    0x3e, 0x71, 0xb0, 0x6c, 0x36, 0x1b, 0xd, 0x86,
    0xc3, 0xe1, 0x80, 0xc0, 0x67, 0xf0, 0x18,

    /* U+0E0F "ฏ" */
    0x3e, 0x71, 0xf0, 0x6c, 0x36, 0x1b, 0xd, 0x86,
    0xc3, 0xe1, 0x80, 0xc3, 0x6f, 0x70,

    /* U+0E10 "ฐ" */
    0x1, 0xff, 0xa0, 0xc0, 0x30, 0x1b, 0xd, 0x86,
    0x46, 0x27, 0x1e, 0x2, 0x63, 0xb7, 0x38,

    /* U+0E11 "ฑ" */
    0xd7, 0x3c, 0xc4, 0x66, 0x36, 0x1b, 0xd, 0x86,
    0xc3, 0x61, 0x80,

    /* U+0E12 "ฒ" */
    0x3f, 0x6, 0xd3, 0xf, 0x6, 0x1e, 0xc, 0x3c,
    0x18, 0x78, 0x30, 0xf0, 0x61, 0xb0, 0xe3, 0x39,
    0xbc,

    /* U+0E13 "ณ" */
    0x3e, 0x6, 0xc6, 0xf, 0x6, 0x1b, 0xc, 0x36,
    0x18, 0x6c, 0x30, 0xd8, 0x61, 0xb0, 0xc7, 0x78,
    0xf6,

    /* U+0E14 "ด" */
    0x3e, 0x31, 0xb0, 0x78, 0x3c, 0x1e, 0xf, 0x6,
    0xc3, 0x39, 0x80,

    /* U+0E15 "ต" */
    0x3f, 0x34, 0xf0, 0x78, 0x3c, 0x1e, 0xf, 0x6,
    0xc3, 0x39, 0x80,

    /* U+0E16 "ถ" */
    0x3e, 0x31, 0xb0, 0x6c, 0x36, 0x1b, 0xd, 0x86,
    0xc3, 0x79, 0x80,

    /* U+0E17 "ท" */
    0xde, 0xe3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3,
    0xc3,

    /* U+0E18 "ธ" */
    0x7e, 0xc0, 0xc0, 0xf8, 0x3e, 0xc7, 0xc3, 0xc3,
    0x7e,

    /* U+0E19 "น" */
    0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc7,
    0x7b,

    /* U+0E1A "บ" */
    0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0x66,
    0x3c,

    /* U+0E1B "ป" */
    0x3, 0x3, 0x3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3,
    0xc3, 0xc3, 0x66, 0x3c,

    /* U+0E1C "ผ" */
    0xe3, 0xc3, 0xc3, 0xdb, 0xdb, 0xe7, 0xe7, 0xe7,
    0xc3,

    /* U+0E1D "ฝ" */
    0x3, 0x3, 0x3, 0xe3, 0xc3, 0xc3, 0xdb, 0xdb,
    0xe7, 0xe7, 0xe7, 0xc3,

    /* U+0E1E "พ" */
    0x8c, 0x79, 0x8b, 0x39, 0x65, 0x65, 0xac, 0xa5,
    0x1c, 0xe1, 0x8c, 0x21, 0x80,

    /* U+0E1F "ฟ" */
    0x0, 0x20, 0xc, 0x1, 0xc6, 0x3c, 0xc5, 0x9c,
    0xb2, 0xb2, 0xd6, 0x52, 0x8e, 0x70, 0xc6, 0x10,
    0xc0,

    /* U+0E20 "ภ" */
    0x3e, 0x71, 0xb0, 0x6c, 0x36, 0x1b, 0xd, 0x86,
    0xc3, 0xe1, 0x80,

    /* U+0E21 "ม" */
    0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xe7,
    0xde,

    /* U+0E22 "ย" */
    0x79, 0xe0, 0xf0, 0x78, 0x33, 0x9e, 0xf, 0x7,
    0xc6, 0x3e, 0x0,

    /* U+0E23 "ร" */
    0x7d, 0x83, 0x7, 0x7, 0xc1, 0xc1, 0xc3, 0xfc,

    /* U+0E24 "ฤ" */
    0x3e, 0x31, 0xf0, 0x6c, 0x36, 0x1b, 0xd, 0x86,
    0xc3, 0x79, 0x80, 0xc0, 0x60, 0x30,

    /* U+0E25 "ล" */
    0x7c, 0x6, 0x3, 0x7f, 0xc3, 0xc3, 0xc3, 0xc3,
    0x73,

    /* U+0E26 "ฦ" */
    0x3e, 0x71, 0xb0, 0x6c, 0x36, 0x1b, 0xd, 0x86,
    0xc3, 0xe1, 0x80, 0xc0, 0x60, 0x30,

    /* U+0E27 "ว" */
    0xfc, 0x86, 0x3, 0x3, 0x3, 0x3, 0x3, 0x86,
    0xfc,

    /* U+0E28 "ศ" */
    0x3f, 0x33, 0x30, 0xd8, 0x6c, 0x37, 0x9b, 0xd,
    0x86, 0xc3, 0x0,

    /* U+0E29 "ษ" */
    0xc3, 0x61, 0xb0, 0xd9, 0xfc, 0x36, 0x1b, 0xc,
    0xcc, 0x3c, 0x0,

    /* U+0E2A "ส" */
    0x7f, 0x3, 0x80, 0xcf, 0xec, 0x36, 0x1b, 0xd,
    0x86, 0x73, 0x0,

    /* U+0E2B "ห" */
    0xc6, 0xcc, 0xdc, 0xf6, 0xe3, 0xc3, 0xc3, 0xc3,
    0xc3,

    /* U+0E2C "ฬ" */
    0x1, 0x81, 0x81, 0x98, 0xfc, 0x1e, 0x4f, 0x27,
    0xab, 0xf7, 0xf1, 0xf8, 0xf8, 0x30,

    /* U+0E2D "อ" */
    0xfc, 0x6, 0x3, 0x3, 0xf3, 0xc3, 0xc3, 0xc6,
    0xfc,

    /* U+0E2E "ฮ" */
    0x3, 0xff, 0x1, 0x80, 0x60, 0x37, 0x9b, 0xd,
    0x86, 0xc6, 0x7e, 0x0,

    /* U+0E2F "ฯ" */
    0x77, 0x8f, 0x1e, 0x37, 0xe0, 0xc1, 0x83, 0x6,

    /* U+0E30 "ะ" */
    0x8f, 0x0, 0x8, 0xf0,

    /* U+0E31 "ั" */
    0xc3, 0xf0,

    /* U+0E32 "า" */
    0xf8, 0x30, 0xc3, 0xc, 0x30, 0xc3, 0xc,

    /* U+0E33 "ำ" */
    0x60, 0x24, 0x6, 0x0, 0x0, 0x7, 0x80, 0x30,
    0xc, 0x3, 0x0, 0xc0, 0x30, 0xc, 0x3, 0x0,
    0xc0,

    /* U+0E34 "ิ" */
    0xfe,

    /* U+0E35 "ี" */
    0x6, 0xf, 0xf8,

    /* U+0E36 "ึ" */
    0xe, 0x17, 0xf8,

    /* U+0E37 "ื" */
    0x12, 0x27, 0xf8,

    /* U+0E38 "ุ" */
    0xed, 0x80,

    /* U+0E39 "ู" */
    0xfb, 0xde,

    /* U+0E3A "ฺ" */
    0xf0,

    /* U+0E3F "฿" */
    0x18, 0x7f, 0x36, 0xdb, 0x6d, 0xb6, 0xdb, 0x6d,
    0xfc, 0xd9, 0xec, 0xf6, 0x7b, 0x3f, 0xf0, 0xc0,
    0x60,

    /* U+0E40 "เ" */
    0xdb, 0x6d, 0xb6, 0xe0,

    /* U+0E41 "แ" */
    0xcd, 0x9b, 0x36, 0x6c, 0xd9, 0xb3, 0x66, 0xee,

    /* U+0E42 "โ" */
    0xfb, 0xc, 0x63, 0x18, 0xc6, 0x31, 0x8c, 0x70,

    /* U+0E43 "ใ" */
    0xf0, 0xc6, 0x67, 0x31, 0x8c, 0x63, 0x18, 0xe0,

    /* U+0E44 "ไ" */
    0xf9, 0x98, 0xc6, 0x31, 0x8c, 0x63, 0x18, 0xe0,

    /* U+0E45 "ๅ" */
    0xf8, 0x30, 0xc3, 0xc, 0x30, 0xc3, 0xc, 0x30,
    0xc3,

    /* U+0E46 "ๆ" */
    0x7e, 0xdb, 0xc3, 0xc3, 0xc3, 0x73, 0x3, 0x3,
    0x3,

    /* U+0E47 "็" */
    0x7e, 0x9, 0x3e,

    /* U+0E48 "่" */
    0xf0,

    /* U+0E49 "้" */
    0xc1, 0xf0,

    /* U+0E4A "๊" */
    0xdb, 0x56, 0x38,

    /* U+0E4B "๋" */
    0x6f, 0x60,

    /* U+0E4C "์" */
    0xfe, 0x0,

    /* U+0E4D "ํ" */
    0x69, 0x96,

    /* U+0E4E "๎" */
    0x6b, 0xce,

    /* U+0E4F "๏" */
    0x3c, 0x5a, 0xa5, 0xa5, 0xa5, 0x99, 0x42, 0x3c,

    /* U+0E50 "๐" */
    0x3e, 0x31, 0xb0, 0x78, 0x3c, 0x1e, 0xd, 0x8c,
    0x7c,

    /* U+0E51 "๑" */
    0x3e, 0x31, 0xb0, 0x78, 0x3c, 0x1f, 0xd, 0xec,
    0xe, 0xe, 0x0, 0x0,

    /* U+0E52 "๒" */
    0xc0, 0x37, 0xef, 0x6f, 0xc3, 0xf0, 0xf7, 0x3c,
    0xd, 0x86, 0x3f, 0x0,

    /* U+0E53 "๓" */
    0x3f, 0x26, 0xf3, 0x79, 0xbc, 0xde, 0xd, 0x86,
    0x73,

    /* U+0E54 "๔" */
    0x1, 0x9f, 0x98, 0x18, 0xc, 0x76, 0x63, 0x30,
    0xd8, 0x3f, 0x80,

    /* U+0E55 "๕" */
    0xc, 0xb, 0xcf, 0xcc, 0xc, 0x6, 0x3b, 0x31,
    0x98, 0x6c, 0x1f, 0xc0,

    /* U+0E56 "๖" */
    0x40, 0x17, 0xc7, 0x19, 0x83, 0x0, 0xc0, 0x30,
    0xc, 0x86, 0x3f, 0x0,

    /* U+0E57 "๗" */
    0x0, 0x37, 0xf3, 0x6d, 0xbc, 0xdb, 0xcd, 0xbc,
    0xdb, 0xc1, 0xb6, 0x1b, 0x39, 0xe0,

    /* U+0E58 "๘" */
    0x1, 0x81, 0x8f, 0xcc, 0xc, 0x36, 0xf, 0x37,
    0x9b, 0xcd, 0xbf, 0x80,

    /* U+0E59 "๙" */
    0x0, 0x5f, 0xf4, 0xdb, 0x36, 0xc4, 0x31, 0xc,
    0x41, 0x98, 0x7a, 0x0,

    /* U+0E5A "๚" */
    0x76, 0x3c, 0x63, 0xc6, 0x3c, 0x63, 0x7f, 0xf0,
    0x63, 0x6, 0x30, 0x63, 0x6, 0x30,

    /* U+0E5B "๛" */
    0x3e, 0x0, 0x33, 0x80, 0x30, 0xfd, 0x18, 0x7e,
    0xbd, 0x3e, 0xa6, 0xf5, 0x41, 0x84, 0x0, 0x7c,
    0x0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 95, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 54, .box_w = 2, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 4, .adv_w = 87, .box_w = 4, .box_h = 3, .ofs_x = 1, .ofs_y = 8},
    {.bitmap_index = 6, .adv_w = 195, .box_w = 11, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 22, .adv_w = 144, .box_w = 8, .box_h = 15, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 37, .adv_w = 227, .box_w = 13, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 55, .adv_w = 198, .box_w = 11, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 71, .adv_w = 45, .box_w = 2, .box_h = 3, .ofs_x = 1, .ofs_y = 8},
    {.bitmap_index = 72, .adv_w = 84, .box_w = 4, .box_h = 15, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 80, .adv_w = 84, .box_w = 4, .box_h = 15, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 88, .adv_w = 112, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 92, .adv_w = 105, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 96, .adv_w = 71, .box_w = 3, .box_h = 3, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 98, .adv_w = 105, .box_w = 5, .box_h = 1, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 99, .adv_w = 47, .box_w = 2, .box_h = 1, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 100, .adv_w = 122, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 109, .adv_w = 172, .box_w = 9, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 122, .adv_w = 93, .box_w = 5, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 129, .adv_w = 149, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 140, .adv_w = 147, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 151, .adv_w = 154, .box_w = 9, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 164, .adv_w = 146, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 175, .adv_w = 156, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 186, .adv_w = 140, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 197, .adv_w = 154, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 208, .adv_w = 156, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 219, .adv_w = 51, .box_w = 2, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 221, .adv_w = 71, .box_w = 3, .box_h = 10, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 225, .adv_w = 139, .box_w = 8, .box_h = 8, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 233, .adv_w = 134, .box_w = 7, .box_h = 4, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 237, .adv_w = 139, .box_w = 8, .box_h = 8, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 245, .adv_w = 132, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 255, .adv_w = 233, .box_w = 13, .box_h = 12, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 275, .adv_w = 191, .box_w = 10, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 289, .adv_w = 166, .box_w = 9, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 302, .adv_w = 182, .box_w = 10, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 316, .adv_w = 180, .box_w = 10, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 330, .adv_w = 145, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 341, .adv_w = 141, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 352, .adv_w = 183, .box_w = 10, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 366, .adv_w = 179, .box_w = 9, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 379, .adv_w = 60, .box_w = 2, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 382, .adv_w = 95, .box_w = 5, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 389, .adv_w = 172, .box_w = 9, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 402, .adv_w = 140, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 413, .adv_w = 213, .box_w = 11, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 429, .adv_w = 176, .box_w = 9, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 442, .adv_w = 211, .box_w = 12, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 459, .adv_w = 154, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 470, .adv_w = 209, .box_w = 12, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 487, .adv_w = 159, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 498, .adv_w = 144, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 509, .adv_w = 150, .box_w = 9, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 522, .adv_w = 178, .box_w = 9, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 535, .adv_w = 191, .box_w = 10, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 549, .adv_w = 256, .box_w = 14, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 569, .adv_w = 187, .box_w = 10, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 583, .adv_w = 179, .box_w = 10, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 597, .adv_w = 158, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 608, .adv_w = 78, .box_w = 4, .box_h = 14, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 615, .adv_w = 121, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 624, .adv_w = 78, .box_w = 4, .box_h = 14, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 631, .adv_w = 140, .box_w = 7, .box_h = 6, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 637, .adv_w = 125, .box_w = 6, .box_h = 1, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 638, .adv_w = 68, .box_w = 3, .box_h = 3, .ofs_x = 1, .ofs_y = 9},
    {.bitmap_index = 640, .adv_w = 146, .box_w = 8, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 648, .adv_w = 169, .box_w = 9, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 662, .adv_w = 140, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 669, .adv_w = 169, .box_w = 9, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 683, .adv_w = 156, .box_w = 9, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 692, .adv_w = 90, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 700, .adv_w = 169, .box_w = 9, .box_h = 12, .ofs_x = 1, .ofs_y = -4},
    {.bitmap_index = 714, .adv_w = 154, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 726, .adv_w = 64, .box_w = 2, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 729, .adv_w = 64, .box_w = 4, .box_h = 15, .ofs_x = -1, .ofs_y = -4},
    {.bitmap_index = 737, .adv_w = 146, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 749, .adv_w = 59, .box_w = 2, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 752, .adv_w = 222, .box_w = 12, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 764, .adv_w = 154, .box_w = 8, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 772, .adv_w = 163, .box_w = 9, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 781, .adv_w = 169, .box_w = 9, .box_h = 12, .ofs_x = 1, .ofs_y = -4},
    {.bitmap_index = 795, .adv_w = 169, .box_w = 9, .box_h = 12, .ofs_x = 1, .ofs_y = -4},
    {.bitmap_index = 809, .adv_w = 102, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 814, .adv_w = 125, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 821, .adv_w = 96, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 828, .adv_w = 154, .box_w = 8, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 836, .adv_w = 150, .box_w = 8, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 844, .adv_w = 209, .box_w = 11, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 855, .adv_w = 158, .box_w = 8, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 863, .adv_w = 150, .box_w = 9, .box_h = 12, .ofs_x = 0, .ofs_y = -4},
    {.bitmap_index = 877, .adv_w = 128, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 883, .adv_w = 88, .box_w = 4, .box_h = 14, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 890, .adv_w = 49, .box_w = 2, .box_h = 16, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 894, .adv_w = 88, .box_w = 4, .box_h = 14, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 901, .adv_w = 141, .box_w = 7, .box_h = 2, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 903, .adv_w = 164, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 914, .adv_w = 149, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 923, .adv_w = 151, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 932, .adv_w = 164, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 941, .adv_w = 164, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 950, .adv_w = 159, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 959, .adv_w = 158, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 970, .adv_w = 150, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 981, .adv_w = 156, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 990, .adv_w = 150, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 999, .adv_w = 151, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1008, .adv_w = 255, .box_w = 15, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1025, .adv_w = 257, .box_w = 15, .box_h = 11, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 1046, .adv_w = 169, .box_w = 9, .box_h = 13, .ofs_x = 1, .ofs_y = -4},
    {.bitmap_index = 1061, .adv_w = 169, .box_w = 9, .box_h = 12, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 1075, .adv_w = 155, .box_w = 9, .box_h = 13, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 1090, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1101, .adv_w = 266, .box_w = 15, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1118, .adv_w = 255, .box_w = 15, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1135, .adv_w = 173, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1146, .adv_w = 173, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1157, .adv_w = 164, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1168, .adv_w = 161, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1177, .adv_w = 143, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1186, .adv_w = 161, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1195, .adv_w = 165, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1204, .adv_w = 165, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1216, .adv_w = 165, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1225, .adv_w = 165, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1237, .adv_w = 199, .box_w = 11, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1250, .adv_w = 194, .box_w = 11, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1267, .adv_w = 169, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1278, .adv_w = 161, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1287, .adv_w = 162, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1298, .adv_w = 131, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1306, .adv_w = 164, .box_w = 9, .box_h = 12, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 1320, .adv_w = 155, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1329, .adv_w = 169, .box_w = 9, .box_h = 12, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 1343, .adv_w = 147, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1352, .adv_w = 165, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1363, .adv_w = 168, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1374, .adv_w = 159, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1385, .adv_w = 156, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1394, .adv_w = 168, .box_w = 9, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1408, .adv_w = 156, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1417, .adv_w = 158, .box_w = 9, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1429, .adv_w = 135, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1437, .adv_w = 85, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 1441, .adv_w = 0, .box_w = 6, .box_h = 2, .ofs_x = -5, .ofs_y = 10},
    {.bitmap_index = 1443, .adv_w = 108, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1450, .adv_w = 109, .box_w = 10, .box_h = 13, .ofs_x = -4, .ofs_y = 0},
    {.bitmap_index = 1467, .adv_w = 0, .box_w = 7, .box_h = 1, .ofs_x = -8, .ofs_y = 10},
    {.bitmap_index = 1468, .adv_w = 0, .box_w = 7, .box_h = 3, .ofs_x = -7, .ofs_y = 10},
    {.bitmap_index = 1471, .adv_w = 0, .box_w = 7, .box_h = 3, .ofs_x = -7, .ofs_y = 10},
    {.bitmap_index = 1474, .adv_w = 0, .box_w = 7, .box_h = 3, .ofs_x = -8, .ofs_y = 10},
    {.bitmap_index = 1477, .adv_w = 0, .box_w = 3, .box_h = 3, .ofs_x = -3, .ofs_y = -4},
    {.bitmap_index = 1479, .adv_w = 0, .box_w = 5, .box_h = 3, .ofs_x = -6, .ofs_y = -4},
    {.bitmap_index = 1481, .adv_w = 0, .box_w = 2, .box_h = 2, .ofs_x = -3, .ofs_y = -3},
    {.bitmap_index = 1482, .adv_w = 166, .box_w = 9, .box_h = 15, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 1499, .adv_w = 76, .box_w = 3, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1503, .adv_w = 142, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1511, .adv_w = 84, .box_w = 5, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1519, .adv_w = 77, .box_w = 5, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1527, .adv_w = 84, .box_w = 5, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1535, .adv_w = 108, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 1544, .adv_w = 153, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1553, .adv_w = 0, .box_w = 6, .box_h = 4, .ofs_x = -7, .ofs_y = 10},
    {.bitmap_index = 1556, .adv_w = 0, .box_w = 2, .box_h = 2, .ofs_x = -3, .ofs_y = 10},
    {.bitmap_index = 1557, .adv_w = 0, .box_w = 6, .box_h = 2, .ofs_x = -5, .ofs_y = 10},
    {.bitmap_index = 1559, .adv_w = 0, .box_w = 7, .box_h = 3, .ofs_x = -8, .ofs_y = 10},
    {.bitmap_index = 1562, .adv_w = 0, .box_w = 4, .box_h = 3, .ofs_x = -4, .ofs_y = 10},
    {.bitmap_index = 1564, .adv_w = 0, .box_w = 5, .box_h = 2, .ofs_x = -5, .ofs_y = 10},
    {.bitmap_index = 1566, .adv_w = 0, .box_w = 4, .box_h = 4, .ofs_x = -4, .ofs_y = 10},
    {.bitmap_index = 1568, .adv_w = 0, .box_w = 3, .box_h = 5, .ofs_x = -5, .ofs_y = 10},
    {.bitmap_index = 1570, .adv_w = 153, .box_w = 8, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1578, .adv_w = 164, .box_w = 9, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1587, .adv_w = 161, .box_w = 9, .box_h = 10, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 1599, .adv_w = 185, .box_w = 10, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1611, .adv_w = 170, .box_w = 9, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1620, .adv_w = 161, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1631, .adv_w = 161, .box_w = 9, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1643, .adv_w = 167, .box_w = 10, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1655, .adv_w = 214, .box_w = 12, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1669, .adv_w = 169, .box_w = 9, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1681, .adv_w = 185, .box_w = 10, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1693, .adv_w = 215, .box_w = 12, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1707, .adv_w = 291, .box_w = 17, .box_h = 8, .ofs_x = 1, .ofs_y = 0}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/



/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 95, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 3585, .range_length = 58, .glyph_id_start = 96,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 3647, .range_length = 29, .glyph_id_start = 154,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    }
};

/*-----------------
 *    KERNING
 *----------------*/


/*Map glyph_ids to kern left classes*/
static const uint8_t kern_left_class_mapping[] =
{
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 0, 2,
    0, 3, 0, 0, 0, 4, 5, 0,
    6, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 7, 8, 9, 10, 11, 12,
    13, 14, 15, 16, 17, 18, 19, 20,
    21, 22, 23, 24, 25, 26, 27, 28,
    29, 30, 31, 32, 0, 0, 0, 0,
    0, 0, 33, 34, 35, 36, 37, 38,
    39, 40, 41, 42, 43, 44, 40, 40,
    45, 46, 47, 48, 49, 50, 51, 52,
    53, 54, 55, 56, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0
};

/*Map glyph_ids to kern right classes*/
static const uint8_t kern_right_class_mapping[] =
{
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 0, 0,
    0, 2, 3, 0, 0, 4, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 5, 6, 7, 8, 9, 0,
    7, 0, 10, 11, 12, 0, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22,
    23, 24, 25, 26, 0, 0, 0, 0,
    0, 0, 27, 28, 29, 30, 31, 32,
    33, 34, 35, 36, 37, 38, 39, 39,
    40, 41, 30, 42, 43, 44, 45, 46,
    47, 48, 49, 50, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0
};

/*Kern values between classes*/
static const int8_t kern_class_values[] =
{
    0, -6, -12, -14, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -17, 0, -12, 0, 0,
    -12, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -8, 0, -17, -9, 0,
    -12, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, -24, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -17, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, -12, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -6, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, -24, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -12, 0, 0, 0,
    -9, 0, -14, 0, 0, 0, -1, 0,
    -6, 0, -17, 0, -17, 0, -12, -32,
    -17, -35, -27, -12, -35, -4, -9, 0,
    -12, -12, 0, 0, 0, 0, 0, -4,
    0, 0, 0, -17, 0, 0, -9, -17,
    -9, -22, -22, -12, -22, -9, -14, 0,
    0, 0, -17, 0, -4, 0, 0, 0,
    0, 0, 0, 0, -6, 0, -6, 0,
    -12, -6, 0, -19, -6, -12, -19, -5,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -4, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -9, -9, -9, -12, -9,
    -12, 0, 0, 0, -6, 0, -6, 0,
    0, 0, 0, 0, 0, 0, -6, 0,
    -6, 0, 0, -1, 0, -9, -9, -9,
    -12, 0, -4, 0, -4, -4, 0, -4,
    0, 0, 0, -6, 0, 0, 0, -6,
    0, -6, -6, -6, -6, -12, -12, -9,
    -14, -1, -14, 0, 0, 0, -17, 0,
    -6, -4, 0, 0, -12, 0, 0, 0,
    0, 0, 0, 0, -12, -14, 0, -17,
    -14, -19, -24, -12, -8, 0, -8, -8,
    0, -6, 0, 0, 0, -12, 0, 0,
    0, -8, 0, 0, -12, 0, -6, -12,
    -12, -14, -9, -9, 0, 0, 0, 0,
    0, 0, -4, 0, 0, 0, 0, 0,
    0, 0, -4, 0, -4, 0, 0, 0,
    0, -1, -4, -1, -6, 0, -4, 0,
    -6, -6, 0, -9, 0, 0, 0, -1,
    0, 0, 0, -6, 0, 0, -4, -6,
    -4, -17, -17, -6, -12, 0, -32, 0,
    0, 0, -27, 0, -4, 0, 0, 0,
    -19, 0, -1, 0, -6, 0, -6, 0,
    -1, 0, 0, -1, -1, -1, -1, 0,
    -26, 0, -19, -19, -18, -13, -18, 0,
    -9, -12, 0, -9, -17, -19, -19, -12,
    -19, -9, -22, -24, -24, -26, -24, -17,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -9, 0, -9, 0, 0,
    -12, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -9, 0, 0,
    -9, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -6, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, -9, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -6, -6, -6, -9, 0, -12, 0,
    0, 0, -14, 0, -8, 0, 0, 0,
    -1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, -6,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -8, 0, -9, 0, 0, 0, 0,
    -12, 0, 0, 0, -12, 0, -22, 0,
    0, 0, -9, 0, 0, -9, -22, 0,
    -22, 0, -9, -9, -8, -9, -8, -9,
    -17, -6, -17, -6, -22, -22, 0, -17,
    0, -6, -9, -12, 0, -6, -6, -22,
    -12, -12, -17, -17, -14, -27, -24, -17,
    -27, -14, 0, 0, 0, 0, -4, 0,
    -14, 0, -4, 0, 0, 0, 0, 0,
    -14, 0, -14, 0, 0, -22, -9, -32,
    -24, -6, -32, -4, -1, 0, -6, -6,
    -10, -6, 0, 0, 0, 0, 0, 0,
    0, -10, 0, 0, -6, -17, 0, -24,
    -24, 0, -19, 0, 0, 0, 0, 0,
    -6, 0, -9, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, -4,
    0, -9, -9, -9, -9, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, -4,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, -12, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -4, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -12, -12, -12, -12, 0,
    -1, 0, 0, 0, -17, 0, -4, 0,
    0, 0, -6, 0, 0, 0, -1, 0,
    0, 0, -6, -14, 0, -17, -17, -19,
    -22, -9, -4, 0, -4, -4, -4, -4,
    0, 0, 0, -9, 0, 0, 0, -4,
    0, 0, -6, -9, 0, -12, -12, -12,
    -12, 0, -24, 0, 0, 0, -26, 0,
    -6, 0, 0, 0, -19, 0, 0, 0,
    -4, -4, -4, 0, -6, -6, 0, -12,
    -9, -14, -14, -9, -14, 0, -14, -14,
    -14, 0, -14, 0, 0, -17, 0, 0,
    0, -14, 0, 0, -12, -9, -9, -12,
    -12, -12, -12, -12, 0, 0, 0, 0,
    -17, 0, -4, 0, 0, 0, -6, 0,
    0, 0, 0, 0, 0, 0, -6, -14,
    0, -17, -17, -19, -22, -9, -4, 0,
    -4, -4, -4, -4, 0, 0, 0, -9,
    0, 0, 0, -4, 0, 0, -6, -9,
    0, -12, -12, -12, -12, 0, -9, 0,
    0, 0, -12, 0, -4, 0, 0, 0,
    -4, 0, 0, 0, -4, 0, -4, 0,
    0, -9, 0, -17, -8, -6, -17, -6,
    -10, 0, -10, -10, -10, 0, -10, 0,
    0, 0, 0, 0, 0, -10, 0, 0,
    -9, -9, -9, -14, -14, -12, -14, -12,
    0, 0, 0, 0, -12, 0, -6, 0,
    0, 0, -6, 0, 0, 0, 0, 0,
    0, 0, -9, -6, 0, -12, -12, -6,
    -17, -6, -4, 0, -4, -4, 0, -12,
    0, 0, 0, 0, 0, 0, 0, -4,
    0, 0, -4, -9, -4, -12, -12, -12,
    -12, -4, -24, 0, 0, 0, -32, 0,
    -14, 0, 0, 0, -26, 0, -4, 0,
    -14, 0, -14, 0, -6, 9, 0, 0,
    -4, -6, -3, -4, -29, -6, -29, -29,
    -29, -17, -29, 0, -6, -4, -4, -6,
    -27, -29, -22, -19, -29, -17, -24, -29,
    -29, -29, -32, -29, 0, 0, 0, 0,
    -17, 0, 0, 0, 0, 0, -4, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -6, -9, -6, -12, 0,
    -12, -12, 0, 0, 0, 0, 0, -9,
    0, 0, 0, -6, 0, 0, -9, 0,
    -4, -12, -12, -12, -12, -9, -24, 0,
    0, 0, -35, -5, -17, 0, 0, 0,
    -27, 0, -9, 0, -17, 0, -17, -9,
    -19, 0, 0, 0, -9, -6, -9, -6,
    -27, -9, -27, -27, -27, -12, -27, -9,
    -17, -17, -9, -9, -22, -27, -22, -22,
    -24, -19, -22, -27, -27, -27, -27, -27,
    -24, 0, 0, 0, -27, 0, -17, 0,
    0, 0, -19, 0, -9, 0, -17, 0,
    -17, -9, -12, -4, 0, -9, -4, -9,
    -9, -6, -19, 0, -19, -19, -19, -17,
    -19, -9, -9, -17, 0, 0, -9, -19,
    -17, -12, -17, -17, -14, -17, -17, -22,
    -17, -17, -6, 0, 0, 0, -12, 0,
    -19, 0, 0, 0, -6, 0, -9, 0,
    -19, 0, -19, -6, -12, 0, -6, -6,
    -9, 0, -9, -9, -17, -6, -17, -17,
    -17, -17, 0, 0, -6, 0, 0, -6,
    -6, -17, -6, -12, -17, -17, -12, -31,
    -24, -12, -22, -17, -42, 0, 0, 0,
    -35, 0, -22, 0, 0, 0, -32, -9,
    -9, -12, -22, -6, -22, -6, -17, -6,
    0, -9, -9, -9, -9, -9, -32, 0,
    -35, -32, -35, -24, -35, -12, -14, -17,
    0, 0, -19, -37, -27, -24, -32, -17,
    -27, -32, -32, -32, -27, -27, 0, 0,
    0, 0, -6, 0, -12, 0, 0, 0,
    0, 0, 0, -9, -9, 0, -9, 0,
    -1, -4, -9, -6, -9, -9, -6, -4,
    -12, 0, -12, -12, -12, -17, 0, 0,
    0, -12, 0, 0, 0, -12, 0, 0,
    -6, -17, 0, -17, -17, -12, -19, 0,
    -12, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -24, 0, -17, -9, 0,
    -26, 0, 0, 0, -5, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, -6,
    0, 0, -4, -6, 0, -12, -14, -6,
    -12, 0, -12, 0, 0, 0, -12, 0,
    0, -8, 0, 0, -12, 0, 0, 0,
    0, 0, 0, 0, 0, -24, 0, -24,
    -17, -19, -35, 0, -6, 0, -4, -6,
    0, -9, 0, 0, 0, -4, 0, 0,
    0, -6, 0, 0, -4, -9, -6, -17,
    -17, -17, -17, -9, -1, 0, 0, 0,
    -9, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, -24,
    0, -17, -12, -12, -19, 0, -1, 0,
    -4, -4, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -4, 0, 0, -1, 0,
    0, -12, -9, -6, -6, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -6, 0, 0, 0, 0, -9, 0,
    -4, 0, 0, -4, 0, 0, 0, 0,
    0, -6, 0, 0, 0, -4, 0, 0,
    0, 0, 0, -6, -6, -9, -12, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, -10, 0, 0, 0, 0, 0,
    0, 0, 0, -24, 0, -17, -17, -12,
    -19, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -10, -8, -5,
    -8, 0, -19, 0, 0, 0, -19, 0,
    0, 0, 0, 0, -27, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -17, 0, -12, -17,
    0, -6, 0, 0, -1, -9, 0, -6,
    -4, -9, -9, -1, -6, -9, -14, -12,
    -17, -17, -12, -9, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, -19,
    0, -9, 0, 0, -24, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -6, 0, 0, 0, 0,
    0, -12, -14, -9, -9, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -27, 0, -17, -9, -6, -24, 0,
    -4, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -4, 0, 0,
    0, 0, 0, -12, -6, -12, -12, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -6, 0, 0, -9, -6,
    -14, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, -14,
    -8, 0, -12, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, -12,
    0, 0, 0, 0, -12, 0, 0, 0,
    -6, 0, 0, 0, 0, 0, -12, 0,
    0, 0, -12, 0, -12, 0, -9, -22,
    0, -22, -8, 0, -17, 0, -14, 0,
    -17, -17, 0, 0, -13, 0, 0, -9,
    0, 0, 0, -17, 0, 0, -9, -6,
    -9, -12, -12, -12, -12, -6, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -6, 0, -9, 0, -6, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -12, 0, 0, 0, -17, 0, 0, 0,
    0, 0, -9, 0, 0, 0, -4, 0,
    -4, 0, -6, -29, 0, -27, -19, -17,
    -37, 0, -6, -4, -4, -4, 0, -9,
    0, 0, 0, -4, 0, 0, 0, -6,
    0, 0, -6, -4, 0, -17, -17, -17,
    -17, -12, 0, 0, 0, 0, -12, 0,
    0, -8, 0, 0, -12, 0, 0, 0,
    0, 0, 0, 0, 0, -24, 0, -24,
    -17, -19, -35, 0, -6, 0, -4, -6,
    0, -9, 0, 0, 0, -4, 0, 0,
    0, -6, 0, 0, -4, -9, -6, -17,
    -17, -17, -17, -9, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, -29,
    0, -17, 0, 0, -17, 0, 0, 0,
    -4, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -9, -9, -9, -9, 0, -24, 0,
    0, 0, -17, 0, 0, 0, 0, 0,
    -22, 0, 0, 0, 0, 0, 0, 0,
    0, -19, 0, -14, -9, -15, -24, -9,
    -12, 0, -1, -4, -8, -1, -10, 0,
    0, -4, 0, 0, 0, -4, 0, -1,
    -4, 0, -1, -1, -1, -1, -4, -1,
    0, 0, 0, 0, -9, 0, 0, 0,
    0, 0, -6, 0, 0, 0, 0, 0,
    0, 0, 0, -27, 0, -17, -12, -14,
    -27, 0, -4, 0, -6, -4, 0, 0,
    0, 0, 0, 0, 0, 0, 0, -4,
    0, 0, -6, -4, 0, -12, -12, -17,
    -14, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -9, 0, -9, 0, 0, -17, 0, -17,
    -14, -9, -19, 0, -4, 0, -1, -6,
    0, -6, -8, 0, 0, -1, 0, 0,
    0, -1, 0, 0, -5, -6, -1, -9,
    -6, -4, -10, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, -17,
    0, -12, -12, 0, -19, 0, -4, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -6, -6, -9, -9, 0, -24, 0,
    0, 0, -22, -9, 0, -12, 0, -6,
    -24, 0, 0, -12, -12, 0, -12, 0,
    -9, -29, -12, -27, -17, -27, -32, -17,
    -12, 0, -17, -17, -10, -9, -10, 0,
    0, -6, 0, 0, -6, -17, -9, 0,
    -12, -6, -6, -6, -12, -12, -12, -12,
    -24, 0, 0, 0, -22, -9, 0, -12,
    0, -6, -22, 0, 0, -12, -12, 0,
    -12, 0, -12, -29, -12, -27, -17, -24,
    -32, -17, -14, 0, -17, -17, -8, -12,
    -10, 0, 0, -6, 0, 0, -6, -17,
    -6, -4, -12, -4, -9, -6, -9, -17,
    -12, -6, -12, 0, 0, 0, -12, -9,
    0, -12, 0, -6, -17, 0, 0, -12,
    -12, 0, -12, 0, -12, -29, -12, -27,
    -22, -12, -32, -12, -12, 0, -17, -17,
    -5, 0, -10, 0, 0, 0, 0, 0,
    0, -17, 0, 0, -12, -6, -9, -12,
    -12, -6, -9, -9, -17, 0, 0, 0,
    -19, -12, 0, 0, 0, -6, -24, 0,
    0, -12, -12, 0, -12, 0, -12, -32,
    -12, -27, -17, -22, -27, -22, -17, 0,
    -12, -14, -8, -1, -10, 0, 0, -4,
    0, 0, 0, -17, -6, 0, -12, 0,
    -6, -12, -6, -9, -6, -9, -4, 0,
    0, 0, -9, 0, 0, 0, 0, 0,
    -1, 0, 0, 0, 0, 0, 0, 0,
    0, -29, -9, -27, -17, -17, -27, 0,
    -6, 0, -6, -14, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -12, 0, 0,
    -6, -4, -6, -12, -6, -9, -9, -6
};


/*Collect the kern class' data in one place*/
static const lv_font_fmt_txt_kern_classes_t kern_classes =
{
    .class_pair_values   = kern_class_values,
    .left_class_mapping  = kern_left_class_mapping,
    .right_class_mapping = kern_right_class_mapping,
    .left_class_cnt      = 56,
    .right_class_cnt     = 50,
};

/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = &kern_classes,
    .kern_scale = 16,
    .cmap_num = 3,
    .bpp = 1,
    .kern_classes = 1,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif
};



/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t lv_font_prompt_16 = {
#else
lv_font_t lv_font_prompt_16 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 19,          /*The maximum line height required by the font*/
    .base_line = 4,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -1,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if LV_FONT_PROMPT_16*/

