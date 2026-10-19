#pragma once

/**
 * LabVIEW serial link over the board's USB port (USB Serial/JTAG, e.g. COM20).
 *
 * ASCII, one command per line (terminated by \n or \r), case-insensitive.
 * Every reply is one line terminated by \n. Log output is kept off this port,
 * so the line after a command is always that command's reply.
 *
 *   R      -> <roll>                             e.g. "4": cached TRNG roll 1-6, triggers a reroll
 *   PING   -> PONG                               connection check
 *   STATS  -> STATS,<T total>,<T1>..<T6>,<P total>,<P1>..<P6>   on-board roller counts
 *   RESET  -> OK,RESET                           zeroes the on-board roller counts
 *   CPU    -> CPU,<core0 %>,<core1 %>            e.g. CPU,12.3,4.0
 *   other  -> ERR,UNKNOWN,<command>
 *
 * Start roll_cache_start() and dice_tasks_start() first.
 */
void serial_link_start(void);
