#pragma once

#include <stdint.h>

// Seed used by the Core 1 roller task. A fixed seed means the sequence is
// identical after every reset - the hallmark of a PRNG.
#define PRNG_DEFAULT_SEED 0x2545F491u

/**
 * One independent generator. Each task that needs pseudo random numbers owns
 * its own prng_t, so tasks never share (or race on) generator state.
 */
typedef struct {
    uint32_t state;
} prng_t;

/** Set the generator state. A seed of 0 is replaced with PRNG_DEFAULT_SEED. */
void prng_init(prng_t *p, uint32_t seed);

/** Next 32-bit pseudo random value. */
uint32_t prng_u32(prng_t *p);

/** Unbiased pseudo random die roll, 1-6. */
uint8_t prng_roll_d6(prng_t *p);
