#pragma once

#include <stdint.h>

/**
 * Enable the hardware entropy source so esp_random() returns true random
 * numbers without Wi-Fi/Bluetooth running. Call once at startup.
 *
 * Note: the entropy source uses the SAR ADC; call bootloader_random_disable()
 * before using the ADC or starting Wi-Fi/BT.
 */
void trng_init(void);

/** 32 true random bits from the hardware RNG. */
uint32_t trng_u32(void);

/** Unbiased true random die roll, 1-6. */
uint8_t trng_roll_d6(void);
