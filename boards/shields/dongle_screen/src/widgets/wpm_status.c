/*
 * Copyright (c) 2024 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/wpm_state_changed.h>

#include "wpm_status.h"
#include <fonts.h>

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

#define WPM_BAR_WIDTH  10
#define WPM_BAR_HEIGHT 144
#define WPM_MAX        120

static lv_color_t wpm_bar_buffer[WPM_BAR_WIDTH * WPM_BAR_HEIGHT];

static void draw_wpm_bar(lv_obj_t *canvas, int wpm)
{
    if (wpm < 0)        wpm = 0;
    if (wpm > WPM_MAX)  wpm = WPM_MAX;

    int fill = (wpm * WPM_BAR_HEIGHT) / WPM_MAX;

    lv_canvas_fill_bg(canvas, lv_color_black(), LV_OPA_COVER);

    for (int y = 0; y < fill; y++) {
        for (int x = 0; x < WPM_BAR_WIDTH; x++) {
            lv_canvas_set_px(canvas, x, y, lv_color_hex(0xf09537), LV_OPA_COVER);
        }
    }
}

struct wpm_status_state
{
    int wpm;
};

static struct wpm_status_state get_state(const zmk_event_t *_eh)
{
    const struct zmk_wpm_state_changed *ev = as_zmk_wpm_state_changed(_eh);

    return (struct wpm_status_state){
        .wpm = ev ? ev->state : 0};
}

static void set_wpm(struct zmk_widget_wpm_status *widget, struct wpm_status_state state)
{

    char wpm_text[12];
    snprintf(wpm_text, sizeof(wpm_text), "%i", state.wpm);
    lv_label_set_text(widget->wpm_label, wpm_text);
    draw_wpm_bar(widget->wpm_bar, state.wpm);
}

static void wpm_status_update_cb(struct wpm_status_state state)
{
    struct zmk_widget_wpm_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node)
    {
        set_wpm(widget, state);
    }
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_wpm_status, struct wpm_status_state,
                            wpm_status_update_cb, get_state)
ZMK_SUBSCRIPTION(widget_wpm_status, zmk_wpm_state_changed);

// output_status.c
int zmk_widget_wpm_status_init(struct zmk_widget_wpm_status *widget, lv_obj_t *parent)
{
    widget->obj = lv_obj_create(parent);
    lv_obj_set_size(widget->obj, 240, 168);

    widget->wpm_label = lv_label_create(widget->obj);
    lv_obj_align(widget->wpm_label, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_text_color(widget->wpm_label, lv_color_hex(0x965fd4), 0);

    widget->wpm_bar = lv_canvas_create(widget->obj);
    lv_canvas_set_buffer(widget->wpm_bar, wpm_bar_buffer, WPM_BAR_WIDTH, WPM_BAR_HEIGHT, LV_COLOR_FORMAT_RGB565);
    lv_obj_align(widget->wpm_bar, LV_ALIGN_TOP_LEFT, 2, 28);
    draw_wpm_bar(widget->wpm_bar, 0);

    // Only here as a sample
    // widget->font_test = lv_label_create(widget->obj);
    // lv_obj_set_style_text_font(widget->font_test, &NerdFonts_Regular_20, 0);
    // lv_obj_align(widget->font_test, LV_ALIGN_TOP_RIGHT, -80, 0);

    // Only here as a sample
    // lv_label_set_text(widget->font_test, "󰕓󰘳󰘵󰘶");
    // TODO: Explizit als UTF-8 wert setzen?

    sys_slist_append(&widgets, &widget->node);

    widget_wpm_status_init();
    return 0;
}

lv_obj_t *zmk_widget_wpm_status_obj(struct zmk_widget_wpm_status *widget)
{
    return widget->obj;
}
