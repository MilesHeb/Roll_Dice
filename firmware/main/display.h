#pragma once

#define DISPLAY_TASK_STACK    8192
#define DISPLAY_TASK_PRIORITY 2   // below the roller and serial tasks; drawing can wait a few ms

/** Power up the AMOLED, initialise LVGL and register the display driver. */
void display_init(void);

/**
 * Start the LVGL task, not pinned to either core. Create all LVGL objects
 * before calling this; after it, only the LVGL task may call LVGL.
 */
void display_start(void);
