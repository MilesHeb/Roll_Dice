// Roll cache: holds exactly one TRNG + PRNG roll pair until LabVIEW asks for it.
//
// The cache *is* a FreeRTOS queue of depth 1 (producer/consumer):
//   - roll_cache_task (producer) rolls a pair and xQueueSend()s it. The queue
//     is then full, so the next xQueueSend() blocks: the task sleeps, using no
//     CPU, for as long as the pair sits in the cache.
//   - roll_cache_take() (consumer, called by the serial link) xQueueReceive()s
//     the pair. That frees the slot, which unblocks the producer to roll the
//     replacement immediately.
// No flags, polling or extra semaphores: the queue does the signalling and is
// safe to use from any task on either core.

#include "roll_cache.h"

#include "esp_log.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "pseudo_rng.h"
#include "true_rng.h"

#define ROLL_CACHE_TASK_STACK    2560
#define ROLL_CACHE_TASK_PRIORITY 6  // above the rollers so a released pair is refilled at once

static const char *TAG = "roll_cache";

static QueueHandle_t s_cache;

static void roll_cache_task(void *arg)
{
    // Own PRNG instance: the Core 1 roller's sequence stays untouched
    prng_t prng;
    prng_init(&prng, ROLL_CACHE_PRNG_SEED);
    uint32_t seq = 0;

    while (1) {
        roll_pair_t pair = {
            .seq = ++seq,
            .trng = trng_roll_d6(),
            .prng = prng_roll_d6(&prng),
        };
        // Blocks here while the previous pair is still cached
        xQueueSend(s_cache, &pair, portMAX_DELAY);
        ESP_LOGD(TAG, "cached #%lu: TRNG %u, PRNG %u", (unsigned long)pair.seq, pair.trng, pair.prng);
    }
}

void roll_cache_start(void)
{
    s_cache = xQueueCreate(1, sizeof(roll_pair_t));
    configASSERT(s_cache);
    BaseType_t ok = xTaskCreate(roll_cache_task, "roll_cache", ROLL_CACHE_TASK_STACK, NULL,
                                ROLL_CACHE_TASK_PRIORITY, NULL);
    configASSERT(ok == pdPASS);
}

bool roll_cache_take(roll_pair_t *out, TickType_t timeout)
{
    return xQueueReceive(s_cache, out, timeout) == pdTRUE;
}

bool roll_cache_peek(roll_pair_t *out)
{
    return xQueuePeek(s_cache, out, 0) == pdTRUE;
}
