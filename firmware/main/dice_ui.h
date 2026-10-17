#pragma once

/**
 * Build the dice statistics screen: one bar chart per RNG source showing each
 * face's share of all rolls, refreshed periodically from dice_get_stats().
 * Call after display_init() and from the LVGL task only.
 */
void dice_ui_create(void);
