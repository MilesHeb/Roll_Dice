#pragma once

/**
 * LabVIEW serial link over the board's USB port (USB Serial/JTAG, e.g. COM20).
 *
 * ASCII, one command per line (terminated by \n or \r), case-insensitive.
 * Every reply is one comma-separated line terminated by \n. The same port
 * also carries ESP_LOG output, so skip lines that don't start with a reply
 * prefix (PONG, ROLL, STATS, OK, CPU, ERR).
 *
 *   PING   -> PONG,Dice_Roll
 *   ROLL   -> ROLL,<seq>,<trng>,<prng>          releases the cached pair, triggers a reroll
 *   STATS  -> STATS,<T total>,<T1>..<T6>,<P total>,<P1>..<P6>
 *   RESET  -> OK,RESET                           zeroes both rolling counts
 *   CPU    -> CPU,<core0 %>,<core1 %>            e.g. CPU,12.3,4.0
 *   other  -> ERR,UNKNOWN,<command>
 *
 * Start roll_cache_start() and dice_tasks_start() first.
 */
void serial_link_start(void);
