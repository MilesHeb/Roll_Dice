#pragma once

/** Power up the AMOLED, initialise LVGL and register the display driver. */
void display_init(void);

/** Run the LVGL timer handler forever. Never returns. */
void display_run(void);
