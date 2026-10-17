#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"

// Seed for the cache's own PRNG instance (independent of the Core 1 roller)
#define ROLL_CACHE_PRNG_SEED 0x9E3779B9u

/** One cached pair of rolls, released to LabVIEW on request. */
typedef struct {
    uint32_t seq;   // increments by 1 for every pair rolled, starting at 1
    uint8_t trng;   // true random roll, 1-6
    uint8_t prng;   // pseudo random roll, 1-6
} roll_pair_t;

/** Create the cache and start the refill task. Call trng_init() first. */
void roll_cache_start(void);

/**
 * Release the cached pair to the caller and trigger a reroll.
 * Blocks up to `timeout` if the next pair isn't ready yet.
 * @return true if a pair was released
 */
bool roll_cache_take(roll_pair_t *out, TickType_t timeout);

/** Look at the cached pair without releasing it. @return false if empty */
bool roll_cache_peek(roll_pair_t *out);
