/*******************************************************************************
 * Size: 12 px
 * Bpp: 1
 * Opts: --font Prompt-Regular.ttf -r 0x20-0x7E,0xE01-0xE5B --size 12 --bpp 1 --format lvgl --lv-include lvgl.h --output frontend/src/ui/fonts/lv_font_prompt_12.c --lv-font-name lv_font_prompt_12
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl.h"
#endif

#ifndef LV_FONT_PROMPT_12
#define LV_FONT_PROMPT_12 1
#endif

#if LV_FONT_PROMPT_12

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xfd,

    /* U+0022 "\"" */
    0xb4,

    /* U+0023 "#" */
    0x22, 0x22, 0xff, 0x64, 0x44, 0xfe, 0x48, 0x88,

    /* U+0024 "$" */
    0x23, 0xe9, 0x4e, 0x38, 0xe5, 0x2f, 0x88,

    /* U+0025 "%" */
    0x63, 0x24, 0x89, 0x41, 0xb0, 0xb, 0x85, 0x11,
    0x44, 0x8e,

    /* U+0026 "&" */
    0x30, 0x48, 0x48, 0x30, 0x52, 0x8a, 0x86, 0x7b,

    /* U+0027 "'" */
    0xc0,

    /* U+0028 "(" */
    0x4a, 0x49, 0x24, 0x89, 0x0,

    /* U+0029 ")" */
    0x48, 0x92, 0x49, 0x69, 0x0,

    /* U+002A "*" */
    0x27, 0xdc, 0xa0,

    /* U+002B "+" */
    0x44, 0xf4,

    /* U+002C "," */
    0xe0,

    /* U+002D "-" */
    0xf0,

    /* U+002E "." */
    0x80,

    /* U+002F "/" */
    0x13, 0x22, 0x44, 0xc8, 0x80,

    /* U+0030 "0" */
    0x38, 0x8a, 0xc, 0x18, 0x30, 0x51, 0x1c,

    /* U+0031 "1" */
    0x7c, 0x92, 0x49,

    /* U+0032 "2" */
    0x72, 0x20, 0x82, 0x10, 0x84, 0x3f,

    /* U+0033 "3" */
    0xf0, 0x42, 0xe0, 0x84, 0x3e,

    /* U+0034 "4" */
    0x8, 0x62, 0x92, 0x4b, 0xf0, 0x82,

    /* U+0035 "5" */
    0x7d, 0x7, 0x83, 0x4, 0x18, 0xde,

    /* U+0036 "6" */
    0x39, 0xf, 0xb3, 0x86, 0x1c, 0xde,

    /* U+0037 "7" */
    0xfc, 0x30, 0x86, 0x10, 0xc2, 0x18,

    /* U+0038 "8" */
    0x7a, 0x18, 0x5e, 0x86, 0x18, 0x5e,

    /* U+0039 "9" */
    0x7a, 0x38, 0x63, 0x7c, 0x10, 0x9c,

    /* U+003A ":" */
    0x84,

    /* U+003B ";" */
    0xc0, 0x38,

    /* U+003C "<" */
    0xc, 0xee, 0x30, 0x38, 0x30,

    /* U+003D "=" */
    0xf8, 0x3e,

    /* U+003E ">" */
    0x81, 0x81, 0x87, 0x72, 0x0,

    /* U+003F "?" */
    0xf0, 0x42, 0x26, 0x20, 0x8,

    /* U+0040 "@" */
    0x1f, 0x18, 0x24, 0xe6, 0x49, 0x92, 0x64, 0x98,
    0xda, 0x0, 0x60, 0xf, 0x80,

    /* U+0041 "A" */
    0x10, 0x50, 0xa3, 0x64, 0x4f, 0xa0, 0xc1,

    /* U+0042 "B" */
    0xfa, 0x18, 0x7e, 0x86, 0x18, 0x7e,

    /* U+0043 "C" */
    0x3e, 0x41, 0x80, 0x80, 0x80, 0x80, 0x41, 0x3e,

    /* U+0044 "D" */
    0xf9, 0xa, 0xc, 0x18, 0x30, 0x61, 0x7c,

    /* U+0045 "E" */
    0xfc, 0x21, 0xe8, 0x42, 0x1f,

    /* U+0046 "F" */
    0xfc, 0x21, 0xe8, 0x42, 0x10,

    /* U+0047 "G" */
    0x3e, 0x82, 0x4, 0x78, 0x30, 0x50, 0x9f,

    /* U+0048 "H" */
    0x83, 0x6, 0xf, 0xf8, 0x30, 0x60, 0xc1,

    /* U+0049 "I" */
    0xff,

    /* U+004A "J" */
    0x24, 0x92, 0x4e,

    /* U+004B "K" */
    0x8d, 0x32, 0xc7, 0xa, 0x12, 0x22, 0x42,

    /* U+004C "L" */
    0x84, 0x21, 0x8, 0x42, 0x1f,

    /* U+004D "M" */
    0x81, 0xc3, 0xc3, 0xa5, 0xa5, 0x99, 0x99, 0x99,

    /* U+004E "N" */
    0x83, 0x87, 0x8d, 0x99, 0xb1, 0xe1, 0xc1,

    /* U+004F "O" */
    0x3e, 0x31, 0xa0, 0x30, 0x18, 0xc, 0x5, 0xc,
    0x7c,

    /* U+0050 "P" */
    0xfa, 0x18, 0x61, 0xfa, 0x8, 0x20,

    /* U+0051 "Q" */
    0x3e, 0x31, 0xa0, 0x30, 0x18, 0xc, 0x5, 0x4,
    0x7f,

    /* U+0052 "R" */
    0xfa, 0x18, 0x61, 0xfa, 0x68, 0xa1,

    /* U+0053 "S" */
    0x7c, 0x20, 0xc1, 0x4, 0x3e,

    /* U+0054 "T" */
    0xfc, 0x41, 0x4, 0x10, 0x41, 0x4,

    /* U+0055 "U" */
    0x83, 0x6, 0xc, 0x18, 0x30, 0x71, 0xbe,

    /* U+0056 "V" */
    0x83, 0xd, 0x12, 0x26, 0xc5, 0xe, 0x8,

    /* U+0057 "W" */
    0x8c, 0x63, 0x1c, 0xcd, 0x7a, 0x52, 0x9c, 0xe3,
    0x30, 0x84,

    /* U+0058 "X" */
    0x86, 0x88, 0xa1, 0x83, 0x8d, 0x11, 0x43,

    /* U+0059 "Y" */
    0xc6, 0x89, 0xb1, 0x41, 0x2, 0x4, 0x8,

    /* U+005A "Z" */
    0xfc, 0x21, 0x84, 0x21, 0x8c, 0x3f,

    /* U+005B "[" */
    0xea, 0xaa, 0xac,

    /* U+005C "\\" */
    0x88, 0xc4, 0x42, 0x23, 0x10,

    /* U+005D "]" */
    0xd5, 0x55, 0x5c,

    /* U+005E "^" */
    0x23, 0xb5, 0x10,

    /* U+005F "_" */
    0xf8,

    /* U+0060 "`" */
    0xa0,

    /* U+0061 "a" */
    0x70, 0x5f, 0x18, 0xbc,

    /* U+0062 "b" */
    0x82, 0x8, 0x2e, 0xce, 0x18, 0x73, 0xb8,

    /* U+0063 "c" */
    0x7b, 0x18, 0x20, 0xc5, 0xe0,

    /* U+0064 "d" */
    0x4, 0x10, 0x5f, 0xce, 0x18, 0x73, 0x74,

    /* U+0065 "e" */
    0x7a, 0x1f, 0xe0, 0xc1, 0xf0,

    /* U+0066 "f" */
    0x34, 0x4e, 0x44, 0x44, 0x40,

    /* U+0067 "g" */
    0x77, 0x38, 0x61, 0xcd, 0xd0, 0x43, 0x78,

    /* U+0068 "h" */
    0x82, 0x8, 0x2e, 0xc6, 0x18, 0x61, 0x84,

    /* U+0069 "i" */
    0xbf,

    /* U+006A "j" */
    0x20, 0x92, 0x49, 0x27, 0x80,

    /* U+006B "k" */
    0x84, 0x21, 0x3b, 0x72, 0x96, 0x98,

    /* U+006C "l" */
    0xff, 0x80,

    /* U+006D "m" */
    0xb7, 0x64, 0x62, 0x31, 0x18, 0x8c, 0x44,

    /* U+006E "n" */
    0xbb, 0x18, 0x61, 0x86, 0x10,

    /* U+006F "o" */
    0x7b, 0x38, 0x61, 0xcd, 0xe0,

    /* U+0070 "p" */
    0xbb, 0x38, 0x61, 0xcf, 0xe8, 0x20, 0x80,

    /* U+0071 "q" */
    0x77, 0x38, 0x61, 0xcd, 0xf0, 0x41, 0x4,

    /* U+0072 "r" */
    0xbc, 0x88, 0x88,

    /* U+0073 "s" */
    0x7c, 0x38, 0x71, 0xf8,

    /* U+0074 "t" */
    0x44, 0xe4, 0x44, 0x47,

    /* U+0075 "u" */
    0x86, 0x18, 0x61, 0x8d, 0xd0,

    /* U+0076 "v" */
    0x8c, 0x54, 0xa7, 0x10,

    /* U+0077 "w" */
    0x99, 0x99, 0xdb, 0x6a, 0x66, 0x66,

    /* U+0078 "x" */
    0xc9, 0x43, 0x1c, 0x5a, 0x20,

    /* U+0079 "y" */
    0x8e, 0x24, 0x96, 0x70, 0xc2, 0x8, 0xc0,

    /* U+007A "z" */
    0xf8, 0x8c, 0xc4, 0x7c,

    /* U+007B "{" */
    0x69, 0x25, 0x12, 0x49, 0x80,

    /* U+007C "|" */
    0xff, 0xf0,

    /* U+007D "}" */
    0xc9, 0x24, 0x52, 0x4b, 0x0,

    /* U+007E "~" */
    0xe0, 0x70,

    /* U+0E01 "ก" */
    0x3c, 0x85, 0xa, 0x14, 0x28, 0x50, 0x80,

    /* U+0E02 "ข" */
    0xc5, 0x14, 0x61, 0x86, 0x17, 0x80,

    /* U+0E03 "ฃ" */
    0xca, 0x73, 0x18, 0xc5, 0xc0,

    /* U+0E04 "ค" */
    0x7a, 0x18, 0x79, 0x86, 0x18, 0x40,

    /* U+0E05 "ฅ" */
    0xfe, 0x98, 0x79, 0x86, 0x18, 0x40,

    /* U+0E06 "ฆ" */
    0xc5, 0x1c, 0x61, 0x87, 0x1b, 0x80,

    /* U+0E07 "ง" */
    0x3a, 0x3c, 0x51, 0x45, 0xb3, 0x80,

    /* U+0E08 "จ" */
    0xf8, 0x30, 0x51, 0x45, 0x23, 0x0,

    /* U+0E09 "ฉ" */
    0xf8, 0x30, 0x61, 0x86, 0x37, 0x40,

    /* U+0E0A "ช" */
    0xda, 0x97, 0x18, 0xcd, 0xc0,

    /* U+0E0B "ซ" */
    0xc9, 0x6c, 0x61, 0x86, 0x37, 0x80,

    /* U+0E0C "ฌ" */
    0x38, 0x28, 0x85, 0x10, 0xa2, 0x14, 0x42, 0x8c,
    0x59, 0x70,

    /* U+0E0D "ญ" */
    0x38, 0x28, 0x85, 0x10, 0xa2, 0x14, 0x46, 0x87,
    0xd8, 0x8, 0x1e,

    /* U+0E0E "ฎ" */
    0x7b, 0x14, 0x51, 0x45, 0x1c, 0x41, 0x3d, 0x10,

    /* U+0E0F "ฏ" */
    0x7b, 0x14, 0x51, 0x45, 0x1c, 0x45, 0x6c,

    /* U+0E10 "ฐ" */
    0x7, 0xf0, 0xc1, 0x45, 0x14, 0x8c, 0x15, 0xb0,

    /* U+0E11 "ฑ" */
    0xbc, 0xc5, 0x8a, 0x14, 0x28, 0x50, 0x80,

    /* U+0E12 "ฒ" */
    0x7e, 0x3a, 0x46, 0x8, 0xc1, 0x18, 0x23, 0x86,
    0x5c, 0xb0,

    /* U+0E13 "ณ" */
    0x38, 0x28, 0x85, 0x10, 0xa2, 0x14, 0x42, 0x88,
    0x58, 0xf8,

    /* U+0E14 "ด" */
    0x3c, 0x8e, 0xc, 0x18, 0x38, 0x5c, 0x80,

    /* U+0E15 "ต" */
    0x7f, 0xa6, 0xc, 0x18, 0x38, 0x5c, 0x80,

    /* U+0E16 "ถ" */
    0x3c, 0x85, 0xa, 0x14, 0x28, 0x5c, 0x80,

    /* U+0E17 "ท" */
    0xbb, 0x18, 0x61, 0x86, 0x18, 0x40,

    /* U+0E18 "ธ" */
    0x7c, 0x30, 0x68, 0xc5, 0xc0,

    /* U+0E19 "น" */
    0x86, 0x18, 0x61, 0x86, 0x37, 0x40,

    /* U+0E1A "บ" */
    0x86, 0x18, 0x61, 0x87, 0x37, 0x80,

    /* U+0E1B "ป" */
    0x4, 0x18, 0x61, 0x86, 0x18, 0x73, 0x78,

    /* U+0E1C "ผ" */
    0xc6, 0x1b, 0x6d, 0xcf, 0x38, 0x40,

    /* U+0E1D "ฝ" */
    0x4, 0x1c, 0x61, 0xb6, 0xdc, 0xf3, 0x84,

    /* U+0E1E "พ" */
    0x48, 0xa6, 0xd5, 0x4a, 0xa3, 0x51, 0x98, 0x88,

    /* U+0E1F "ฟ" */
    0x0, 0x80, 0x52, 0x29, 0xa5, 0x52, 0xa8, 0xd4,
    0x64, 0x22, 0x0,

    /* U+0E20 "ภ" */
    0x7b, 0x14, 0x51, 0x45, 0x1c, 0x40,

    /* U+0E21 "ม" */
    0x86, 0x18, 0x61, 0x87, 0x1b, 0x80,

    /* U+0E22 "ย" */
    0x66, 0x18, 0x59, 0x86, 0x17, 0x80,

    /* U+0E23 "ร" */
    0x7c, 0x20, 0xe0, 0x87, 0xc0,

    /* U+0E24 "ฤ" */
    0x3c, 0x85, 0xa, 0x14, 0x28, 0x5c, 0x81, 0x2,

    /* U+0E25 "ล" */
    0x78, 0x10, 0x5f, 0x86, 0x16, 0x40,

    /* U+0E26 "ฦ" */
    0x7b, 0x14, 0x51, 0x45, 0x1c, 0x41, 0x4,

    /* U+0E27 "ว" */
    0x7a, 0x30, 0x41, 0x6, 0x37, 0x80,

    /* U+0E28 "ศ" */
    0x7e, 0x18, 0x79, 0x86, 0x18, 0x40,

    /* U+0E29 "ษ" */
    0x85, 0xa, 0x3c, 0x28, 0x59, 0x9e, 0x0,

    /* U+0E2A "ส" */
    0x7c, 0x8, 0x13, 0xe8, 0x50, 0x99, 0x0,

    /* U+0E2B "ห" */
    0x8e, 0x6b, 0xf1, 0x86, 0x18, 0x40,

    /* U+0E2C "ฬ" */
    0x4, 0x28, 0xe1, 0xb6, 0xdc, 0xf3, 0x84,

    /* U+0E2D "อ" */
    0xf8, 0x30, 0x79, 0x86, 0x3f, 0x0,

    /* U+0E2E "ฮ" */
    0x7, 0xf0, 0xc1, 0xe6, 0x18, 0xfc,

    /* U+0E2F "ฯ" */
    0x6c, 0x62, 0xf0, 0x84, 0x20,

    /* U+0E30 "ะ" */
    0x9c, 0x4e,

    /* U+0E31 "ั" */
    0x8f,

    /* U+0E32 "า" */
    0xe1, 0x11, 0x11, 0x10,

    /* U+0E33 "ำ" */
    0xc1, 0x43, 0x0, 0xe0, 0x20, 0x40, 0x81, 0x2,
    0x4,

    /* U+0E34 "ิ" */
    0xf8,

    /* U+0E35 "ี" */
    0xf, 0xc0,

    /* U+0E36 "ึ" */
    0x18, 0xfe,

    /* U+0E37 "ื" */
    0x2f, 0xc0,

    /* U+0E38 "ุ" */
    0xd4,

    /* U+0E39 "ู" */
    0xd5, 0x70,

    /* U+0E3A "ฺ" */
    0x80,

    /* U+0E3F "฿" */
    0x20, 0x8f, 0xa9, 0xa6, 0x9f, 0xa9, 0xa6, 0x9f,
    0x88,

    /* U+0E40 "เ" */
    0xaa, 0xac,

    /* U+0E41 "แ" */
    0x94, 0xa5, 0x29, 0x4b, 0x60,

    /* U+0E42 "โ" */
    0xf4, 0x44, 0x44, 0x44, 0x47,

    /* U+0E43 "ใ" */
    0xe4, 0xa4, 0x92, 0x4c,

    /* U+0E44 "ไ" */
    0xf6, 0x44, 0x44, 0x44, 0x46,

    /* U+0E45 "ๅ" */
    0xe1, 0x11, 0x11, 0x11, 0x10,

    /* U+0E46 "ๆ" */
    0x7e, 0x98, 0x61, 0x64, 0x10, 0x40,

    /* U+0E47 "็" */
    0xfc, 0x3c,

    /* U+0E48 "่" */
    0xc0,

    /* U+0E49 "้" */
    0x87,

    /* U+0E4A "๊" */
    0xfc, 0xe6,

    /* U+0E4B "๋" */
    0xe8,

    /* U+0E4C "์" */
    0xf8,

    /* U+0E4D "ํ" */
    0xdb, 0x0,

    /* U+0E4E "๎" */
    0x6a, 0x60,

    /* U+0E4F "๏" */
    0x7b, 0x2b, 0x73, 0xcf, 0xe7, 0x80,

    /* U+0E50 "๐" */
    0x7b, 0x38, 0x61, 0xcd, 0xe0,

    /* U+0E51 "๑" */
    0x7a, 0x38, 0x61, 0x74, 0x23, 0x0,

    /* U+0E52 "๒" */
    0x81, 0x6e, 0xad, 0x1b, 0xb8, 0xdf, 0x0,

    /* U+0E53 "๓" */
    0x6d, 0x26, 0x4c, 0x98, 0x2c, 0x40,

    /* U+0E54 "๔" */
    0x6, 0xfb, 0x4, 0x69, 0x12, 0x1f, 0x80,

    /* U+0E55 "๕" */
    0x18, 0x54, 0xf6, 0x8, 0xd2, 0x24, 0x3f,

    /* U+0E56 "๖" */
    0x40, 0xf9, 0x18, 0x10, 0x28, 0xcf, 0x0,

    /* U+0E57 "๗" */
    0x0, 0xb7, 0x64, 0xb2, 0x59, 0x2c, 0x15, 0x8c,

    /* U+0E58 "๘" */
    0x2, 0x79, 0x4, 0x8, 0xd0, 0x64, 0xb6,

    /* U+0E59 "๙" */
    0x1, 0x7e, 0x92, 0x9a, 0x88, 0x88, 0x78,

    /* U+0E5A "๚" */
    0x68, 0xc4, 0x62, 0x2f, 0xf0, 0x88, 0x44, 0x22,

    /* U+0E5B "๛" */
    0x70, 0x4, 0x55, 0x22, 0xff, 0x66, 0xac, 0x40,
    0x3c, 0x0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 71, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 40, .box_w = 1, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 2, .adv_w = 65, .box_w = 3, .box_h = 2, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 3, .adv_w = 146, .box_w = 8, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 11, .adv_w = 108, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 18, .adv_w = 170, .box_w = 10, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 28, .adv_w = 148, .box_w = 8, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 36, .adv_w = 34, .box_w = 1, .box_h = 2, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 37, .adv_w = 63, .box_w = 3, .box_h = 11, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 42, .adv_w = 63, .box_w = 3, .box_h = 11, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 47, .adv_w = 84, .box_w = 5, .box_h = 4, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 50, .adv_w = 79, .box_w = 4, .box_h = 4, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 52, .adv_w = 53, .box_w = 2, .box_h = 2, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 53, .adv_w = 79, .box_w = 4, .box_h = 1, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 54, .adv_w = 36, .box_w = 1, .box_h = 1, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 55, .adv_w = 92, .box_w = 4, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 60, .adv_w = 129, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 67, .adv_w = 70, .box_w = 3, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 70, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 76, .adv_w = 110, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 81, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 87, .adv_w = 109, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 93, .adv_w = 117, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 99, .adv_w = 105, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 105, .adv_w = 116, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 111, .adv_w = 117, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 117, .adv_w = 38, .box_w = 1, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 118, .adv_w = 53, .box_w = 2, .box_h = 7, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 120, .adv_w = 104, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 125, .adv_w = 101, .box_w = 5, .box_h = 3, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 127, .adv_w = 104, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 132, .adv_w = 99, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 137, .adv_w = 175, .box_w = 10, .box_h = 10, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 150, .adv_w = 143, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 157, .adv_w = 125, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 163, .adv_w = 136, .box_w = 8, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 171, .adv_w = 135, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 178, .adv_w = 109, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 183, .adv_w = 106, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 188, .adv_w = 137, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 195, .adv_w = 134, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 202, .adv_w = 45, .box_w = 1, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 203, .adv_w = 72, .box_w = 3, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 206, .adv_w = 129, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 213, .adv_w = 105, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 218, .adv_w = 160, .box_w = 8, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 226, .adv_w = 132, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 233, .adv_w = 159, .box_w = 9, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 242, .adv_w = 116, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 248, .adv_w = 157, .box_w = 9, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 257, .adv_w = 120, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 263, .adv_w = 108, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 268, .adv_w = 113, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 274, .adv_w = 134, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 281, .adv_w = 143, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 288, .adv_w = 192, .box_w = 10, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 298, .adv_w = 141, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 305, .adv_w = 134, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 312, .adv_w = 119, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 318, .adv_w = 59, .box_w = 2, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 321, .adv_w = 91, .box_w = 4, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 326, .adv_w = 59, .box_w = 2, .box_h = 11, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 329, .adv_w = 105, .box_w = 5, .box_h = 4, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 332, .adv_w = 94, .box_w = 5, .box_h = 1, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 333, .adv_w = 51, .box_w = 2, .box_h = 2, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 334, .adv_w = 109, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 338, .adv_w = 127, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 345, .adv_w = 105, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 350, .adv_w = 127, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 357, .adv_w = 117, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 362, .adv_w = 68, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 367, .adv_w = 127, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 374, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 381, .adv_w = 48, .box_w = 1, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 382, .adv_w = 48, .box_w = 3, .box_h = 11, .ofs_x = -1, .ofs_y = -3},
    {.bitmap_index = 387, .adv_w = 109, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 393, .adv_w = 44, .box_w = 1, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 395, .adv_w = 166, .box_w = 9, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 402, .adv_w = 115, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 407, .adv_w = 122, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 412, .adv_w = 127, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 419, .adv_w = 127, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 426, .adv_w = 77, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 429, .adv_w = 94, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 433, .adv_w = 72, .box_w = 4, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 437, .adv_w = 115, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 442, .adv_w = 113, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 446, .adv_w = 157, .box_w = 8, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 452, .adv_w = 118, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 457, .adv_w = 113, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 464, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 468, .adv_w = 66, .box_w = 3, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 473, .adv_w = 37, .box_w = 1, .box_h = 12, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 475, .adv_w = 66, .box_w = 3, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 480, .adv_w = 106, .box_w = 6, .box_h = 2, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 482, .adv_w = 123, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 489, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 495, .adv_w = 113, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 500, .adv_w = 123, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 506, .adv_w = 123, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 512, .adv_w = 119, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 518, .adv_w = 119, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 524, .adv_w = 113, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 530, .adv_w = 117, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 536, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 541, .adv_w = 113, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 547, .adv_w = 191, .box_w = 11, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 557, .adv_w = 193, .box_w = 11, .box_h = 8, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 568, .adv_w = 127, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 576, .adv_w = 127, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 583, .adv_w = 116, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 591, .adv_w = 120, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 598, .adv_w = 199, .box_w = 11, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 608, .adv_w = 191, .box_w = 11, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 618, .adv_w = 130, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 625, .adv_w = 130, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 632, .adv_w = 123, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 639, .adv_w = 121, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 645, .adv_w = 107, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 650, .adv_w = 121, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 656, .adv_w = 123, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 662, .adv_w = 123, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 669, .adv_w = 124, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 675, .adv_w = 124, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 682, .adv_w = 149, .box_w = 9, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 690, .adv_w = 145, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 701, .adv_w = 127, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 707, .adv_w = 121, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 713, .adv_w = 121, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 719, .adv_w = 98, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 724, .adv_w = 123, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 732, .adv_w = 117, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 738, .adv_w = 127, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 745, .adv_w = 111, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 751, .adv_w = 123, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 757, .adv_w = 126, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 764, .adv_w = 119, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 771, .adv_w = 117, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 777, .adv_w = 126, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 784, .adv_w = 117, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 790, .adv_w = 119, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 796, .adv_w = 101, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 801, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 803, .adv_w = 0, .box_w = 4, .box_h = 2, .ofs_x = -4, .ofs_y = 8},
    {.bitmap_index = 804, .adv_w = 81, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 808, .adv_w = 81, .box_w = 7, .box_h = 10, .ofs_x = -3, .ofs_y = 0},
    {.bitmap_index = 817, .adv_w = 0, .box_w = 5, .box_h = 1, .ofs_x = -6, .ofs_y = 8},
    {.bitmap_index = 818, .adv_w = 0, .box_w = 5, .box_h = 2, .ofs_x = -6, .ofs_y = 8},
    {.bitmap_index = 820, .adv_w = 0, .box_w = 5, .box_h = 3, .ofs_x = -6, .ofs_y = 8},
    {.bitmap_index = 822, .adv_w = 0, .box_w = 5, .box_h = 2, .ofs_x = -5, .ofs_y = 8},
    {.bitmap_index = 824, .adv_w = 0, .box_w = 2, .box_h = 3, .ofs_x = -3, .ofs_y = -4},
    {.bitmap_index = 825, .adv_w = 0, .box_w = 4, .box_h = 3, .ofs_x = -5, .ofs_y = -4},
    {.bitmap_index = 827, .adv_w = 0, .box_w = 1, .box_h = 1, .ofs_x = -2, .ofs_y = -2},
    {.bitmap_index = 828, .adv_w = 125, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 837, .adv_w = 57, .box_w = 2, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 839, .adv_w = 107, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 844, .adv_w = 63, .box_w = 4, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 849, .adv_w = 58, .box_w = 3, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 853, .adv_w = 63, .box_w = 4, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 858, .adv_w = 81, .box_w = 4, .box_h = 9, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 863, .adv_w = 115, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 869, .adv_w = 0, .box_w = 5, .box_h = 3, .ofs_x = -5, .ofs_y = 8},
    {.bitmap_index = 871, .adv_w = 0, .box_w = 1, .box_h = 2, .ofs_x = -2, .ofs_y = 8},
    {.bitmap_index = 872, .adv_w = 0, .box_w = 4, .box_h = 2, .ofs_x = -4, .ofs_y = 8},
    {.bitmap_index = 873, .adv_w = 0, .box_w = 5, .box_h = 3, .ofs_x = -6, .ofs_y = 8},
    {.bitmap_index = 875, .adv_w = 0, .box_w = 3, .box_h = 2, .ofs_x = -3, .ofs_y = 8},
    {.bitmap_index = 876, .adv_w = 0, .box_w = 4, .box_h = 2, .ofs_x = -4, .ofs_y = 8},
    {.bitmap_index = 877, .adv_w = 0, .box_w = 3, .box_h = 3, .ofs_x = -3, .ofs_y = 8},
    {.bitmap_index = 879, .adv_w = 0, .box_w = 3, .box_h = 4, .ofs_x = -3, .ofs_y = 8},
    {.bitmap_index = 881, .adv_w = 115, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 887, .adv_w = 123, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 892, .adv_w = 121, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 898, .adv_w = 138, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 905, .adv_w = 128, .box_w = 7, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 911, .adv_w = 121, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 918, .adv_w = 121, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 925, .adv_w = 125, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 932, .adv_w = 160, .box_w = 9, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 940, .adv_w = 127, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 947, .adv_w = 138, .box_w = 8, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 954, .adv_w = 161, .box_w = 9, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 962, .adv_w = 218, .box_w = 13, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    /* PUA 0xF709..0xF70E Shifted High Tone Marks & Mai Taikhu & Karan (y_offset shifted +3px) */
    {.bitmap_index = 869, .adv_w = 0, .box_w = 5, .box_h = 3, .ofs_x = -5, .ofs_y = 11},
    {.bitmap_index = 871, .adv_w = 0, .box_w = 1, .box_h = 2, .ofs_x = -2, .ofs_y = 11},
    {.bitmap_index = 872, .adv_w = 0, .box_w = 4, .box_h = 2, .ofs_x = -4, .ofs_y = 11},
    {.bitmap_index = 873, .adv_w = 0, .box_w = 5, .box_h = 3, .ofs_x = -6, .ofs_y = 11},
    {.bitmap_index = 875, .adv_w = 0, .box_w = 3, .box_h = 2, .ofs_x = -3, .ofs_y = 11},
    {.bitmap_index = 876, .adv_w = 0, .box_w = 4, .box_h = 2, .ofs_x = -4, .ofs_y = 11}
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
    },
    {
        .range_start = 0xF709, .range_length = 6, .glyph_id_start = 183,
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
    0, -5, -9, -11, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -12, 0, -9, 0, 0,
    -9, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -6, 0, -12, -7, 0,
    -9, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, -18, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -12, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, -9, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -5, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, -18, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -9, 0, 0, 0,
    -7, 0, -11, 0, 0, 0, -1, 0,
    -5, 0, -12, 0, -12, 0, -9, -24,
    -12, -26, -20, -9, -26, -3, -7, 0,
    -9, -9, 0, 0, 0, 0, 0, -3,
    0, 0, 0, -12, 0, 0, -7, -12,
    -7, -16, -16, -9, -16, -7, -11, 0,
    0, 0, -12, 0, -3, 0, 0, 0,
    0, 0, 0, 0, -5, 0, -5, 0,
    -9, -5, 0, -14, -5, -9, -14, -4,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -3, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -7, -7, -7, -9, -7,
    -9, 0, 0, 0, -5, 0, -5, 0,
    0, 0, 0, 0, 0, 0, -5, 0,
    -5, 0, 0, -1, 0, -7, -7, -7,
    -9, 0, -3, 0, -3, -3, 0, -3,
    0, 0, 0, -5, 0, 0, 0, -5,
    0, -5, -5, -5, -5, -9, -9, -7,
    -11, -1, -11, 0, 0, 0, -12, 0,
    -5, -3, 0, 0, -9, 0, 0, 0,
    0, 0, 0, 0, -9, -11, 0, -12,
    -11, -14, -18, -9, -6, 0, -6, -6,
    0, -5, 0, 0, 0, -9, 0, 0,
    0, -6, 0, 0, -9, 0, -5, -9,
    -9, -11, -7, -7, 0, 0, 0, 0,
    0, 0, -3, 0, 0, 0, 0, 0,
    0, 0, -3, 0, -3, 0, 0, 0,
    0, -1, -3, -1, -5, 0, -3, 0,
    -5, -5, 0, -7, 0, 0, 0, -1,
    0, 0, 0, -5, 0, 0, -3, -5,
    -3, -12, -12, -5, -9, 0, -24, 0,
    0, 0, -20, 0, -3, 0, 0, 0,
    -14, 0, -1, 0, -5, 0, -5, 0,
    -1, 0, 0, -1, -1, -1, -1, 0,
    -19, 0, -14, -14, -13, -10, -13, 0,
    -7, -9, 0, -7, -12, -14, -14, -9,
    -14, -7, -16, -18, -18, -19, -18, -12,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -7, 0, -7, 0, 0,
    -9, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -7, 0, 0,
    -7, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -5, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, -7, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -5, -5, -5, -7, 0, -9, 0,
    0, 0, -11, 0, -6, 0, 0, 0,
    -1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, -5,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -6, 0, -7, 0, 0, 0, 0,
    -9, 0, 0, 0, -9, 0, -16, 0,
    0, 0, -7, 0, 0, -7, -16, 0,
    -16, 0, -7, -7, -6, -7, -6, -7,
    -12, -5, -12, -5, -16, -16, 0, -12,
    0, -5, -7, -9, 0, -5, -5, -16,
    -9, -9, -12, -12, -11, -20, -18, -12,
    -20, -11, 0, 0, 0, 0, -3, 0,
    -11, 0, -3, 0, 0, 0, 0, 0,
    -11, 0, -11, 0, 0, -16, -7, -24,
    -18, -5, -24, -3, -1, 0, -5, -5,
    -8, -5, 0, 0, 0, 0, 0, 0,
    0, -8, 0, 0, -5, -12, 0, -18,
    -18, 0, -14, 0, 0, 0, 0, 0,
    -5, 0, -7, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, -3,
    0, -7, -7, -7, -7, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, -3,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, -9, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -3, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -9, -9, -9, -9, 0,
    -1, 0, 0, 0, -12, 0, -3, 0,
    0, 0, -5, 0, 0, 0, -1, 0,
    0, 0, -5, -11, 0, -12, -12, -14,
    -16, -7, -3, 0, -3, -3, -3, -3,
    0, 0, 0, -7, 0, 0, 0, -3,
    0, 0, -5, -7, 0, -9, -9, -9,
    -9, 0, -18, 0, 0, 0, -19, 0,
    -5, 0, 0, 0, -14, 0, 0, 0,
    -3, -3, -3, 0, -5, -5, 0, -9,
    -7, -11, -11, -7, -11, 0, -11, -11,
    -11, 0, -11, 0, 0, -12, 0, 0,
    0, -11, 0, 0, -9, -7, -7, -9,
    -9, -9, -9, -9, 0, 0, 0, 0,
    -12, 0, -3, 0, 0, 0, -5, 0,
    0, 0, 0, 0, 0, 0, -5, -11,
    0, -12, -12, -14, -16, -7, -3, 0,
    -3, -3, -3, -3, 0, 0, 0, -7,
    0, 0, 0, -3, 0, 0, -5, -7,
    0, -9, -9, -9, -9, 0, -7, 0,
    0, 0, -9, 0, -3, 0, 0, 0,
    -3, 0, 0, 0, -3, 0, -3, 0,
    0, -7, 0, -12, -6, -5, -12, -5,
    -8, 0, -8, -8, -8, 0, -8, 0,
    0, 0, 0, 0, 0, -8, 0, 0,
    -7, -7, -7, -11, -11, -9, -11, -9,
    0, 0, 0, 0, -9, 0, -5, 0,
    0, 0, -5, 0, 0, 0, 0, 0,
    0, 0, -7, -5, 0, -9, -9, -5,
    -12, -5, -3, 0, -3, -3, 0, -9,
    0, 0, 0, 0, 0, 0, 0, -3,
    0, 0, -3, -7, -3, -9, -9, -9,
    -9, -3, -18, 0, 0, 0, -24, 0,
    -11, 0, 0, 0, -19, 0, -3, 0,
    -11, 0, -11, 0, -5, 7, 0, 0,
    -3, -5, -2, -3, -22, -5, -22, -22,
    -22, -12, -22, 0, -5, -3, -3, -5,
    -20, -22, -16, -14, -22, -12, -18, -22,
    -22, -22, -24, -22, 0, 0, 0, 0,
    -12, 0, 0, 0, 0, 0, -3, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -5, -7, -5, -9, 0,
    -9, -9, 0, 0, 0, 0, 0, -7,
    0, 0, 0, -5, 0, 0, -7, 0,
    -3, -9, -9, -9, -9, -7, -18, 0,
    0, 0, -26, -4, -12, 0, 0, 0,
    -20, 0, -7, 0, -12, 0, -12, -7,
    -14, 0, 0, 0, -7, -5, -7, -5,
    -20, -7, -20, -20, -20, -9, -20, -7,
    -12, -12, -7, -7, -16, -20, -16, -16,
    -18, -14, -16, -20, -20, -20, -20, -20,
    -18, 0, 0, 0, -20, 0, -12, 0,
    0, 0, -14, 0, -7, 0, -12, 0,
    -12, -7, -9, -3, 0, -7, -3, -7,
    -7, -5, -14, 0, -14, -14, -14, -12,
    -14, -7, -7, -12, 0, 0, -7, -14,
    -12, -9, -12, -12, -11, -12, -12, -16,
    -12, -12, -5, 0, 0, 0, -9, 0,
    -14, 0, 0, 0, -5, 0, -7, 0,
    -14, 0, -14, -5, -9, 0, -5, -5,
    -7, 0, -7, -7, -12, -5, -12, -12,
    -12, -12, 0, 0, -5, 0, 0, -5,
    -5, -12, -5, -9, -12, -12, -9, -23,
    -18, -9, -16, -12, -32, 0, 0, 0,
    -26, 0, -16, 0, 0, 0, -24, -7,
    -7, -9, -16, -5, -16, -5, -12, -5,
    0, -7, -7, -7, -7, -7, -24, 0,
    -26, -24, -26, -18, -26, -9, -11, -12,
    0, 0, -14, -28, -20, -18, -24, -12,
    -20, -24, -24, -24, -20, -20, 0, 0,
    0, 0, -5, 0, -9, 0, 0, 0,
    0, 0, 0, -7, -7, 0, -7, 0,
    -1, -3, -7, -5, -7, -7, -5, -3,
    -9, 0, -9, -9, -9, -12, 0, 0,
    0, -9, 0, 0, 0, -9, 0, 0,
    -5, -12, 0, -12, -12, -9, -14, 0,
    -9, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -18, 0, -12, -7, 0,
    -19, 0, 0, 0, -4, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, -5,
    0, 0, -3, -5, 0, -9, -11, -5,
    -9, 0, -9, 0, 0, 0, -9, 0,
    0, -6, 0, 0, -9, 0, 0, 0,
    0, 0, 0, 0, 0, -18, 0, -18,
    -12, -14, -26, 0, -5, 0, -3, -5,
    0, -7, 0, 0, 0, -3, 0, 0,
    0, -5, 0, 0, -3, -7, -5, -12,
    -12, -12, -12, -7, -1, 0, 0, 0,
    -7, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, -18,
    0, -12, -9, -9, -14, 0, -1, 0,
    -3, -3, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -3, 0, 0, -1, 0,
    0, -9, -7, -5, -5, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -5, 0, 0, 0, 0, -7, 0,
    -3, 0, 0, -3, 0, 0, 0, 0,
    0, -5, 0, 0, 0, -3, 0, 0,
    0, 0, 0, -5, -5, -7, -9, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, -8, 0, 0, 0, 0, 0,
    0, 0, 0, -18, 0, -12, -12, -9,
    -14, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -8, -6, -4,
    -6, 0, -14, 0, 0, 0, -14, 0,
    0, 0, 0, 0, -20, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -12, 0, -9, -12,
    0, -5, 0, 0, -1, -7, 0, -5,
    -3, -7, -7, -1, -5, -7, -11, -9,
    -12, -12, -9, -7, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, -14,
    0, -7, 0, 0, -18, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -5, 0, 0, 0, 0,
    0, -9, -11, -7, -7, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -20, 0, -12, -7, -5, -18, 0,
    -3, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -3, 0, 0,
    0, 0, 0, -9, -5, -9, -9, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -5, 0, 0, -7, -5,
    -11, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, -11,
    -6, 0, -9, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, -9,
    0, 0, 0, 0, -9, 0, 0, 0,
    -5, 0, 0, 0, 0, 0, -9, 0,
    0, 0, -9, 0, -9, 0, -7, -16,
    0, -16, -6, 0, -12, 0, -11, 0,
    -12, -12, 0, 0, -10, 0, 0, -7,
    0, 0, 0, -12, 0, 0, -7, -5,
    -7, -9, -9, -9, -9, -5, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -5, 0, -7, 0, -5, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -9, 0, 0, 0, -12, 0, 0, 0,
    0, 0, -7, 0, 0, 0, -3, 0,
    -3, 0, -5, -22, 0, -20, -14, -12,
    -28, 0, -5, -3, -3, -3, 0, -7,
    0, 0, 0, -3, 0, 0, 0, -5,
    0, 0, -5, -3, 0, -12, -12, -12,
    -12, -9, 0, 0, 0, 0, -9, 0,
    0, -6, 0, 0, -9, 0, 0, 0,
    0, 0, 0, 0, 0, -18, 0, -18,
    -12, -14, -26, 0, -5, 0, -3, -5,
    0, -7, 0, 0, 0, -3, 0, 0,
    0, -5, 0, 0, -3, -7, -5, -12,
    -12, -12, -12, -7, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, -22,
    0, -12, 0, 0, -12, 0, 0, 0,
    -3, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -7, -7, -7, -7, 0, -18, 0,
    0, 0, -12, 0, 0, 0, 0, 0,
    -16, 0, 0, 0, 0, 0, 0, 0,
    0, -14, 0, -11, -7, -12, -18, -7,
    -9, 0, -1, -3, -6, -1, -8, 0,
    0, -3, 0, 0, 0, -3, 0, -1,
    -3, 0, -1, -1, -1, -1, -3, -1,
    0, 0, 0, 0, -7, 0, 0, 0,
    0, 0, -5, 0, 0, 0, 0, 0,
    0, 0, 0, -20, 0, -12, -9, -11,
    -20, 0, -3, 0, -5, -3, 0, 0,
    0, 0, 0, 0, 0, 0, 0, -3,
    0, 0, -5, -3, 0, -9, -9, -12,
    -11, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -7, 0, -7, 0, 0, -12, 0, -12,
    -11, -7, -14, 0, -3, 0, -1, -5,
    0, -5, -6, 0, 0, -1, 0, 0,
    0, -1, 0, 0, -4, -5, -1, -7,
    -5, -3, -8, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, -12,
    0, -9, -9, 0, -14, 0, -3, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -5, -5, -7, -7, 0, -18, 0,
    0, 0, -16, -7, 0, -9, 0, -5,
    -18, 0, 0, -9, -9, 0, -9, 0,
    -7, -22, -9, -20, -12, -20, -24, -12,
    -9, 0, -12, -12, -8, -7, -8, 0,
    0, -5, 0, 0, -5, -12, -7, 0,
    -9, -5, -5, -5, -9, -9, -9, -9,
    -18, 0, 0, 0, -16, -7, 0, -9,
    0, -5, -16, 0, 0, -9, -9, 0,
    -9, 0, -9, -22, -9, -20, -12, -18,
    -24, -12, -11, 0, -12, -12, -6, -9,
    -8, 0, 0, -5, 0, 0, -5, -12,
    -5, -3, -9, -3, -7, -5, -7, -12,
    -9, -5, -9, 0, 0, 0, -9, -7,
    0, -9, 0, -5, -12, 0, 0, -9,
    -9, 0, -9, 0, -9, -22, -9, -20,
    -16, -9, -24, -9, -9, 0, -12, -12,
    -4, 0, -8, 0, 0, 0, 0, 0,
    0, -12, 0, 0, -9, -5, -7, -9,
    -9, -5, -7, -7, -12, 0, 0, 0,
    -14, -9, 0, 0, 0, -5, -18, 0,
    0, -9, -9, 0, -9, 0, -9, -24,
    -9, -20, -12, -16, -20, -16, -12, 0,
    -9, -11, -6, -1, -8, 0, 0, -3,
    0, 0, 0, -12, -5, 0, -9, 0,
    -5, -9, -5, -7, -5, -7, -3, 0,
    0, 0, -7, 0, 0, 0, 0, 0,
    -1, 0, 0, 0, 0, 0, 0, 0,
    0, -22, -7, -20, -12, -12, -20, 0,
    -5, 0, -5, -11, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -9, 0, 0,
    -5, -3, -5, -9, -5, -7, -7, -5
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
    .cmap_num = 4,
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
const lv_font_t lv_font_prompt_12 = {
#else
lv_font_t lv_font_prompt_12 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 16,          /*The maximum line height required by the font*/
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



#endif /*#if LV_FONT_PROMPT_12*/

