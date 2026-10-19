#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"

/** One cached true random roll, released to LabVIEW on request. */
typedef struct {
    uint32_t seq;   // increments by 1 for every roll made, starting at 1
    uint8_t value;  // true random roll, 1-6
} roll_t;

/** Create the cache and start the refill task. Call trng_init() first. */
void roll_cache_start(void);

/**
 * Release the cached roll to the caller and trigger a reroll.
 * Blocks up to `timeout` if the next roll isn't ready yet.
 * @return true if a roll was released
 */
bool roll_cache_take(roll_t *out, TickType_t timeout);

/** Look at the cached roll without releasing it. @return false if empty */
bool roll_cache_peek(roll_t *out);
