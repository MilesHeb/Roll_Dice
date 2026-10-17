// Pseudo random number generator (PRNG): a software stand-in for LabVIEW's
// Random Number (0-1) function. LabVIEW doesn't publish its exact algorithm,
// so this uses Marsaglia's xorshift32: fully deterministic, period 2^32 - 1,
// and good enough statistically that the roll counts converge towards even.
//
// A prng_t isn't thread safe; each task owns its own instance.

#include "pseudo_rng.h"

void prng_init(prng_t *p, uint32_t seed)
{
    // xorshift gets stuck at 0 forever, so 0 is never a valid state
    p->state = seed ? seed : PRNG_DEFAULT_SEED;
}

uint32_t prng_u32(prng_t *p)
{
    uint32_t x = p->state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    p->state = x;
    return x;
}

uint8_t prng_roll_d6(prng_t *p)
{
    // Same rejection sampling as the TRNG so the only difference between the
    // two dice is the source of randomness, not the mapping to 1-6.
    const uint32_t reject_below = (uint32_t)(-6u) % 6u; // == 2^32 mod 6 == 4
    uint32_t r;
    do {
        r = prng_u32(p);
    } while (r < reject_below);
    return (uint8_t)(r % 6u) + 1;
}
