// Dice statistics screen: two lv_chart bar graphs side by side.
//
// Each bar is a face's share of all rolls, not its raw count. With a fair die
// every share converges to 1/6 (16.7%), which is the chart's middle grid line,
// so "becoming even" shows up as all six bars settling on that line.

#include "dice_ui.h"

#include <stdlib.h>
#include "cpu_usage.h"
#include "dice_tasks.h"
#include "lvgl.h"

#define UI_REFRESH_MS   250
#define CPU_REFRESH_MS  1000  // longer window gives a steadier load reading

// Y axis in hundredths of a percent: 0..33.34%, so the 16.67% ideal is the
// middle grid line
#define Y_MAX           3334
#define IDEAL_SHARE     1667

#define COLOR_BG        lv_color_hex(0x000000)
#define COLOR_PANEL     lv_color_hex(0x10141F)
#define COLOR_GRID      lv_color_hex(0x2A3145)
#define COLOR_TEXT      lv_color_hex(0xE6E9F2)
#define COLOR_MUTED     lv_color_hex(0x8A93AB)
#define COLOR_TRNG      lv_color_hex(0x00C2FF)
#define COLOR_PRNG      lv_color_hex(0xFFB000)

typedef struct {
    dice_source_t src;
    int core;            // core the source's roller task is pinned to
    lv_obj_t *cpu_label;
    lv_obj_t *chart;
    lv_chart_series_t *series;
    lv_obj_t *footer;
    dice_stats_t stats;  // last snapshot
} dice_panel_t;

static dice_panel_t s_panels[DICE_SRC_COUNT];

// Rewrite the chart's default numeric tick labels:
//   X axis: face number 1-6 (default would be 0-5)
//   Y axis: share of rolls as a percentage
static void chart_draw_event_cb(lv_event_t *e)
{
    lv_obj_draw_part_dsc_t *dsc = lv_event_get_draw_part_dsc(e);
    if (!lv_obj_draw_part_check_type(dsc, &lv_chart_class, LV_CHART_DRAW_PART_TICK_LABEL) || !dsc->text) {
        return;
    }

    if (dsc->id == LV_CHART_AXIS_PRIMARY_X && dsc->value >= 0 && dsc->value < 6) {
        lv_snprintf(dsc->text, dsc->text_length, "%d", (int)dsc->value + 1);
    } else if (dsc->id == LV_CHART_AXIS_PRIMARY_Y) {
        int tenths = (dsc->value + 5) / 10;  // hundredths of % -> tenths of %
        lv_snprintf(dsc->text, dsc->text_length, "%d.%d%%", tenths / 10, tenths % 10);
    }
}

static lv_obj_t *create_panel(dice_panel_t *panel, lv_coord_t x, const char *title,
                              lv_color_t accent)
{
    lv_obj_t *box = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(box);
    lv_obj_set_size(box, 262, 206);
    lv_obj_set_pos(box, x, 30);
    lv_obj_set_style_bg_color(box, COLOR_PANEL, 0);
    lv_obj_set_style_bg_opa(box, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(box, 10, 0);
    lv_obj_clear_flag(box, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *label = lv_label_create(box);
    lv_label_set_text(label, title);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(label, accent, 0);
    lv_obj_set_pos(label, 10, 6);

    panel->cpu_label = lv_label_create(box);
    lv_label_set_text_fmt(panel->cpu_label, "Core %d | CPU --.-%%", panel->core);
    lv_obj_set_style_text_font(panel->cpu_label, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(panel->cpu_label, COLOR_MUTED, 0);
    lv_obj_align(panel->cpu_label, LV_ALIGN_TOP_RIGHT, -10, 8);

    // Bar chart: 6 points (faces), one series
    lv_obj_t *chart = lv_chart_create(box);
    lv_obj_set_size(chart, 196, 126);
    lv_obj_set_pos(chart, 54, 32);
    lv_chart_set_type(chart, LV_CHART_TYPE_BAR);
    lv_chart_set_point_count(chart, 6);
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, 0, Y_MAX);
    lv_chart_set_div_line_count(chart, 3, 0);  // top, middle (= 16.7% ideal), bottom
    lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_Y, 4, 0, 3, 1, true, 50);
    lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_X, 0, 0, 6, 1, true, 18);

    lv_obj_set_style_bg_opa(chart, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(chart, 0, 0);
    lv_obj_set_style_pad_all(chart, 0, 0);
    lv_obj_set_style_pad_column(chart, 10, 0);  // gap between bars
    lv_obj_set_style_line_color(chart, COLOR_GRID, 0);
    lv_obj_set_style_radius(chart, 3, LV_PART_ITEMS);
    lv_obj_set_style_text_font(chart, &lv_font_montserrat_12, LV_PART_TICKS);
    lv_obj_set_style_text_color(chart, COLOR_MUTED, LV_PART_TICKS);
    lv_obj_set_style_line_color(chart, COLOR_GRID, LV_PART_TICKS);
    lv_obj_add_event_cb(chart, chart_draw_event_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);

    panel->chart = chart;
    panel->series = lv_chart_add_series(chart, accent, LV_CHART_AXIS_PRIMARY_Y);
    lv_chart_set_all_value(chart, panel->series, 0);

    panel->footer = lv_label_create(box);
    lv_obj_set_style_text_font(panel->footer, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(panel->footer, COLOR_TEXT, 0);
    lv_obj_set_pos(panel->footer, 10, 184);
    lv_label_set_text(panel->footer, "Waiting for rolls...");

    return box;
}

static void update_panel(dice_panel_t *panel)
{
    dice_get_stats(panel->src, &panel->stats);
    const dice_stats_t *st = &panel->stats;

    int max_dev = 0;  // largest distance from 1/6, hundredths of a percent
    for (int i = 0; i < 6; i++) {
        int share = st->total ? (int)((uint64_t)st->counts[i] * 10000 / st->total) : 0;
        lv_chart_set_value_by_id(panel->chart, panel->series, i, share);
        if (st->total && abs(share - IDEAL_SHARE) > max_dev) {
            max_dev = abs(share - IDEAL_SHARE);
        }
    }
    lv_chart_refresh(panel->chart);

    if (st->total) {
        lv_label_set_text_fmt(panel->footer, "Rolls %lu   Max dev %d.%02d%%",
                              (unsigned long)st->total, max_dev / 100, max_dev % 100);
    }
}

static void ui_timer_cb(lv_timer_t *t)
{
    for (int i = 0; i < DICE_SRC_COUNT; i++) {
        update_panel(&s_panels[i]);
    }
}

static void cpu_timer_cb(lv_timer_t *t)
{
    uint16_t load[CPU_USAGE_CORES];
    cpu_usage_sample(load);
    for (int i = 0; i < DICE_SRC_COUNT; i++) {
        dice_panel_t *panel = &s_panels[i];
        lv_label_set_text_fmt(panel->cpu_label, "Core %d | CPU %u.%u%%", panel->core,
                              load[panel->core] / 10, load[panel->core] % 10);
    }
}

void dice_ui_create(void)
{
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, COLOR_BG, 0);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text(title, "Dice Roll  -  True vs Pseudo RNG");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(title, COLOR_TEXT, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 4);

    s_panels[DICE_SRC_TRNG] = (dice_panel_t){.src = DICE_SRC_TRNG, .core = 0};
    s_panels[DICE_SRC_PRNG] = (dice_panel_t){.src = DICE_SRC_PRNG, .core = 1};
    create_panel(&s_panels[DICE_SRC_TRNG], 4, "TRUE RNG", COLOR_TRNG);
    create_panel(&s_panels[DICE_SRC_PRNG], 270, "PSEUDO RNG", COLOR_PRNG);

    lv_timer_create(ui_timer_cb, UI_REFRESH_MS, NULL);
    cpu_usage_sample((uint16_t[CPU_USAGE_CORES]){0});  // start the first measurement window
    lv_timer_create(cpu_timer_cb, CPU_REFRESH_MS, NULL);
}
