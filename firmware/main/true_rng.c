// True random number generator (TRNG) backed by the ESP32-S3 hardware RNG.
//
// esp_random() reads the hardware RNG. It is only a *true* RNG while an
// entropy source is feeding it: either the RF subsystem (Wi-Fi/BT) or the
// internal SAR ADC noise source enabled by bootloader_random_enable().
// This project doesn't use the radio, so trng_init() enables the ADC source.

#include "true_rng.h"

#include "bootloader_random.h"
#include "esp_random.h"

void trng_init(void)
{
    bootloader_random_enable();
}

uint32_t trng_u32(void)
{
    return esp_random();
}

uint8_t trng_roll_d6(void)
{
    // Rejection sampling: 2^32 isn't a multiple of 6, so plain "r % 6" would
    // make 1-4 very slightly more likely than 5-6. Discard the lowest
    // (2^32 mod 6) values so every face has exactly the same probability.
    const uint32_t reject_below = (uint32_t)(-6u) % 6u; // == 2^32 mod 6 == 4
    uint32_t r;
    do {
        r = esp_random();
    } while (r < reject_below);
    return (uint8_t)(r % 6u) + 1;
}
