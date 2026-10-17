// Dice_Roll: true vs. pseudo random dice rolls on the T-Display-S3 AMOLED,
// streamed to LabVIEW over serial.

#include "dice_tasks.h"
#include "dice_ui.h"
#include "display.h"
#include "roll_cache.h"
#include "serial_link.h"
#include "true_rng.h"

void app_main(void)
{
    trng_init();
    display_init();
    dice_ui_create();
    dice_tasks_start();
    roll_cache_start();
    serial_link_start();

    display_run();
}
