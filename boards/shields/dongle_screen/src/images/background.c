/*
 * Custom background image for the YADS dongle screen.
 *
 * This is a placeholder 1×1 black pixel. Replace it with your own image by:
 *
 *   1. Prepare a PNG at 280×240 px (horizontal) or 240×280 px (vertical).
 *   2. Convert it at https://lvgl.io/tools/imageconverter with:
 *        - Color format : CF_TRUE_COLOR
 *        - Output format: C Array
 *        - Check "Swap color bytes" (required — LV_COLOR_16_SWAP is enabled)
 *   3. Copy the generated .c file here, replacing this file entirely.
 *   4. Rename the exported lv_img_dsc_t variable to `dongle_screen_background`.
 *   5. Enable CONFIG_DONGLE_SCREEN_BACKGROUND_IMAGE=y in your dongle .conf.
 *
 * SPDX-License-Identifier: MIT
 */

#include <lvgl.h>

static const uint8_t background_map[] = {
    0x00, 0x00  /* 1×1 black pixel placeholder */
};

const lv_img_dsc_t dongle_screen_background = {
    .header = {
        .cf = LV_IMG_CF_TRUE_COLOR,
        .always_zero = 0,
        .reserved = 0,
        .w = 1,
        .h = 1,
    },
    .data_size = 2,
    .data = background_map,
};
