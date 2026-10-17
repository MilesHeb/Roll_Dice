#pragma once

#include <stdint.h>

#define CPU_USAGE_CORES 2

/**
 * Measure per-core CPU load since the previous call.
 *
 * @param out_tenths  Receives load for core 0 and core 1 in tenths of a
 *                    percent (0-1000). The first call measures since boot.
 *
 * Call from one task only (the UI timer); it owns the measurement window.
 */
void cpu_usage_sample(uint16_t out_tenths[CPU_USAGE_CORES]);

/** Latest result of cpu_usage_sample(), in tenths of a percent. Safe from any task/core. */
void cpu_usage_get_last(uint16_t out_tenths[CPU_USAGE_CORES]);
