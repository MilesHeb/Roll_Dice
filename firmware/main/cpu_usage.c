// Per-core CPU usage from FreeRTOS run-time stats.
//
// Each core has its own idle task, which only runs when nothing else on that
// core is ready. FreeRTOS accumulates how long every task has run (here in
// microseconds, clocked by esp_timer), so over a sample window:
//
//     load = 100% - (idle task run time / wall clock time)
//
// Needs CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS (see sdkconfig.defaults).

#include "cpu_usage.h"

#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static uint64_t s_prev_idle_us[CPU_USAGE_CORES];
static int64_t s_prev_wall_us;

// Last result, shared with other tasks (e.g. the serial link) on either core
static uint16_t s_last_tenths[CPU_USAGE_CORES];
static portMUX_TYPE s_last_lock = portMUX_INITIALIZER_UNLOCKED;

void cpu_usage_sample(uint16_t out_tenths[CPU_USAGE_CORES])
{
    int64_t wall_now = esp_timer_get_time();
    int64_t wall_delta = wall_now - s_prev_wall_us;
    s_prev_wall_us = wall_now;

    for (int core = 0; core < CPU_USAGE_CORES; core++) {
        uint64_t idle_now = ulTaskGetIdleRunTimeCounterForCore(core);
        uint64_t idle_delta = idle_now - s_prev_idle_us[core];
        s_prev_idle_us[core] = idle_now;

        int64_t busy_tenths = 1000;
        if (wall_delta > 0) {
            busy_tenths = 1000 - (int64_t)(idle_delta * 1000 / (uint64_t)wall_delta);
        }
        // Counter and wall clock are read a few microseconds apart, so clamp
        if (busy_tenths < 0) busy_tenths = 0;
        if (busy_tenths > 1000) busy_tenths = 1000;
        out_tenths[core] = (uint16_t)busy_tenths;
    }

    taskENTER_CRITICAL(&s_last_lock);
    for (int core = 0; core < CPU_USAGE_CORES; core++) {
        s_last_tenths[core] = out_tenths[core];
    }
    taskEXIT_CRITICAL(&s_last_lock);
}

void cpu_usage_get_last(uint16_t out_tenths[CPU_USAGE_CORES])
{
    taskENTER_CRITICAL(&s_last_lock);
    for (int core = 0; core < CPU_USAGE_CORES; core++) {
        out_tenths[core] = s_last_tenths[core];
    }
    taskEXIT_CRITICAL(&s_last_lock);
}
