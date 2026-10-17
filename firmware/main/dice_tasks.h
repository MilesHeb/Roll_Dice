#pragma once

#include <stdint.h>

typedef enum {
    DICE_SRC_TRNG = 0,  // hardware true RNG, rolled on Core 0
    DICE_SRC_PRNG = 1,  // software pseudo RNG, rolled on Core 1
    DICE_SRC_COUNT
} dice_source_t;

typedef struct {
    uint32_t counts[6];  // counts[0] = number of 1s ... counts[5] = number of 6s
    uint32_t total;      // total rolls since start/reset
    uint8_t last_roll;   // most recent roll, 1-6 (0 = none yet)
} dice_stats_t;

/** Roll period for both tasks. */
#define DICE_ROLL_PERIOD_MS 1

/**
 * Start the two roller tasks:
 *   - TRNG task pinned to Core 0
 *   - PRNG task pinned to Core 1
 * Call trng_init() first.
 */
void dice_tasks_start(void);

/** Copy a consistent snapshot of one source's rolling counts. Safe from any task/core. */
void dice_get_stats(dice_source_t src, dice_stats_t *out);

/** Zero one source's rolling counts. Safe from any task/core. */
void dice_reset_stats(dice_source_t src);
