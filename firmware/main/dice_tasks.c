// Two FreeRTOS tasks, one per core, each rolling a die every DICE_ROLL_PERIOD_MS and
// keeping a rolling count of every face.
//
//   Core 0: dice_trng_task - true random rolls from the hardware RNG
//   Core 1: dice_prng_task - pseudo random rolls from xorshift32
//
// Concurrency: each task is the only writer of its own stats, but the stats
// are read from other tasks (serial/LabVIEW, display) that may run on the
// other core. A normal FreeRTOS critical section only blocks the local core,
// so each source gets a portMUX spinlock, which also locks out the other core
// for the few instructions it takes to update or copy the counts.

#include "dice_tasks.h"

#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pseudo_rng.h"
#include "true_rng.h"

#define DICE_TASK_STACK    3072
#define DICE_TASK_PRIORITY 5   // above the LVGL loop (priority 1) so rolls stay on time

static const char *TAG = "dice_tasks";

typedef struct {
    dice_stats_t stats;
    portMUX_TYPE lock;
} dice_slot_t;

static dice_slot_t s_slots[DICE_SRC_COUNT] = {
    [DICE_SRC_TRNG] = {.lock = portMUX_INITIALIZER_UNLOCKED},
    [DICE_SRC_PRNG] = {.lock = portMUX_INITIALIZER_UNLOCKED},
};

static void record_roll(dice_source_t src, uint8_t roll)
{
    dice_slot_t *slot = &s_slots[src];
    taskENTER_CRITICAL(&slot->lock);
    slot->stats.counts[roll - 1]++;
    slot->stats.total++;
    slot->stats.last_roll = roll;
    taskEXIT_CRITICAL(&slot->lock);
}

// Shared task body: roll with the given generator on a fixed DICE_ROLL_PERIOD_MS period.
// xTaskDelayUntil() wakes relative to the previous wake time, so the period
// doesn't drift by however long the roll itself took.
static void roll_forever(dice_source_t src, uint8_t (*roll_d6)(void))
{
    ESP_LOGI(TAG, "%s task running on core %d",
             src == DICE_SRC_TRNG ? "TRNG" : "PRNG", xPortGetCoreID());

    TickType_t last_wake = xTaskGetTickCount();
    while (1) {
        record_roll(src, roll_d6());
        xTaskDelayUntil(&last_wake, pdMS_TO_TICKS(DICE_ROLL_PERIOD_MS));
    }
}

static void dice_trng_task(void *arg)
{
    roll_forever(DICE_SRC_TRNG, trng_roll_d6);
}

// Generator owned by the Core 1 task only. Seeding it in the task makes the
// roll sequence identical after every reset.
static prng_t s_task_prng;

static uint8_t task_prng_roll_d6(void)
{
    return prng_roll_d6(&s_task_prng);
}

static void dice_prng_task(void *arg)
{
    prng_init(&s_task_prng, PRNG_DEFAULT_SEED);
    roll_forever(DICE_SRC_PRNG, task_prng_roll_d6);
}

void dice_tasks_start(void)
{
    BaseType_t ok;
    ok = xTaskCreatePinnedToCore(dice_trng_task, "dice_trng", DICE_TASK_STACK, NULL,
                                 DICE_TASK_PRIORITY, NULL, 0);
    configASSERT(ok == pdPASS);
    ok = xTaskCreatePinnedToCore(dice_prng_task, "dice_prng", DICE_TASK_STACK, NULL,
                                 DICE_TASK_PRIORITY, NULL, 1);
    configASSERT(ok == pdPASS);
}

void dice_get_stats(dice_source_t src, dice_stats_t *out)
{
    dice_slot_t *slot = &s_slots[src];
    taskENTER_CRITICAL(&slot->lock);
    *out = slot->stats;
    taskEXIT_CRITICAL(&slot->lock);
}

void dice_reset_stats(dice_source_t src)
{
    dice_slot_t *slot = &s_slots[src];
    taskENTER_CRITICAL(&slot->lock);
    memset(&slot->stats, 0, sizeof(slot->stats));
    taskEXIT_CRITICAL(&slot->lock);
}
