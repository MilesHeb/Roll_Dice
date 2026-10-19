// Minimal LVGL 8.3 setup for the LILYGO T-Display-S3 AMOLED 1.91"
// (taken from the ported LilyGO lvgl_demo)

#include "display.h"

#include <assert.h>
#include "driver/gpio.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"
#include "rm67162.h"

static lv_disp_draw_buf_t draw_buf;
static lv_color_t *buf;

static void disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    lcd_PushColors(area->x1, area->y1, w, h, (uint16_t *)&color_p->full);
    lv_disp_flush_ready(disp);
}

void display_init(void)
{
    // Touch version: IO38 is the screen power enable. Non-touch version: IO38 is the LED.
    gpio_config_t pwr = {
        .pin_bit_mask = 1ULL << PIN_LED,
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&pwr);
    gpio_set_level(PIN_LED, 1);

    rm67162_init();
    lcd_setRotation(1); // landscape, 536 x 240

    lv_init();

    buf = (lv_color_t *)heap_caps_malloc(sizeof(lv_color_t) * LVGL_LCD_BUF_SIZE, MALLOC_CAP_SPIRAM);
    assert(buf);
    lv_disp_draw_buf_init(&draw_buf, buf, NULL, LVGL_LCD_BUF_SIZE);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = EXAMPLE_LCD_H_RES;
    disp_drv.ver_res = EXAMPLE_LCD_V_RES;
    disp_drv.flush_cb = disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);
}

// LVGL 8 renders in a single loop, so its work can't be split across both
// cores at once. What we can do is not tie it to one: the task is created with
// tskNO_AFFINITY, so the scheduler runs it on whichever core is free at each
// context switch, instead of always adding the drawing load to Core 0 (where
// app_main, and so the old loop, was pinned).
// This is the only task that touches LVGL, so no LVGL locking is needed.
static void lvgl_task(void *arg)
{
    while (1) {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(2));
    }
}

void display_start(void)
{
    BaseType_t ok = xTaskCreatePinnedToCore(lvgl_task, "lvgl", DISPLAY_TASK_STACK, NULL,
                                            DISPLAY_TASK_PRIORITY, NULL, tskNO_AFFINITY);
    configASSERT(ok == pdPASS);
}
