// Roll cache: holds exactly one true random roll until LabVIEW asks for it.
//
// The cache *is* a FreeRTOS queue of depth 1 (producer/consumer):
//   - roll_cache_task (producer) rolls the hardware TRNG and xQueueSend()s the
//     result. The queue is then full, so the next xQueueSend() blocks: the task
//     sleeps, using no CPU, for as long as the roll sits in the cache.
//   - roll_cache_take() (consumer, called by the serial link) xQueueReceive()s
//     the roll. That frees the slot, which unblocks the producer to roll the
//     replacement immediately.
// No flags, polling or extra semaphores: the queue does the signalling and is
// safe to use from any task on either core.

#include "roll_cache.h"

#include "esp_log.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "true_rng.h"

#define ROLL_CACHE_TASK_STACK    2560
#define ROLL_CACHE_TASK_PRIORITY 6  // above the rollers so a released roll is refilled at once

static const char *TAG = "roll_cache";

static QueueHandle_t s_cache;

static void roll_cache_task(void *arg)
{
    uint32_t seq = 0;

    while (1) {
        roll_t roll = {
            .seq = ++seq,
            .value = trng_roll_d6(),
        };
        // Blocks here while the previous roll is still cached
        xQueueSend(s_cache, &roll, portMAX_DELAY);
        ESP_LOGD(TAG, "cached #%lu: %u", (unsigned long)roll.seq, roll.value);
    }
}

void roll_cache_start(void)
{
    s_cache = xQueueCreate(1, sizeof(roll_t));
    configASSERT(s_cache);
    BaseType_t ok = xTaskCreate(roll_cache_task, "roll_cache", ROLL_CACHE_TASK_STACK, NULL,
                                ROLL_CACHE_TASK_PRIORITY, NULL);
    configASSERT(ok == pdPASS);
}

bool roll_cache_take(roll_t *out, TickType_t timeout)
{
    return xQueueReceive(s_cache, out, timeout) == pdTRUE;
}

bool roll_cache_peek(roll_t *out)
{
    return xQueuePeek(s_cache, out, 0) == pdTRUE;
}
