// LabVIEW serial link: a request/response command protocol on the USB
// Serial/JTAG port. See serial_link.h for the command list.
//
// The port carries protocol traffic only: ESP_LOG output is kept off it via
// CONFIG_ESP_CONSOLE_SECONDARY_NONE (logs go to UART0 instead). So every line
// LabVIEW reads after sending "R" is the roll itself, and LabVIEW needs no
// filtering - one VISA Read per command.
//
// One task owns the port. It blocks in usb_serial_jtag_read_bytes() (no CPU
// used while LabVIEW is quiet), assembles bytes into a line, and answers each
// complete line. All data it reports comes through the thread-safe APIs of the
// other modules, so it can run on either core.

#include "serial_link.h"

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include "cpu_usage.h"
#include "dice_tasks.h"
#include "driver/usb_serial_jtag.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "roll_cache.h"

#define SERIAL_TASK_STACK    4096
#define SERIAL_TASK_PRIORITY 4
#define SERIAL_LINE_MAX      32
#define SERIAL_REPLY_MAX     160
#define SERIAL_WRITE_TIMEOUT pdMS_TO_TICKS(100)  // don't block forever if LabVIEW closed the port

static void send_line(const char *line)
{
    usb_serial_jtag_write_bytes(line, strlen(line), SERIAL_WRITE_TIMEOUT);
}

static void reply_stats(char *out, size_t size)
{
    dice_stats_t t, p;
    dice_get_stats(DICE_SRC_TRNG, &t);
    dice_get_stats(DICE_SRC_PRNG, &p);
    snprintf(out, size, "STATS,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu\n",
             (unsigned long)t.total,
             (unsigned long)t.counts[0], (unsigned long)t.counts[1], (unsigned long)t.counts[2],
             (unsigned long)t.counts[3], (unsigned long)t.counts[4], (unsigned long)t.counts[5],
             (unsigned long)p.total,
             (unsigned long)p.counts[0], (unsigned long)p.counts[1], (unsigned long)p.counts[2],
             (unsigned long)p.counts[3], (unsigned long)p.counts[4], (unsigned long)p.counts[5]);
}

static void handle_command(const char *cmd)
{
    char reply[SERIAL_REPLY_MAX];

    if (strcasecmp(cmd, "R") == 0) {
        // The one command LabVIEW needs: release the cached TRNG roll (which
        // triggers a reroll) and reply with just the digit, e.g. "4\n"
        roll_t roll;
        if (roll_cache_take(&roll, pdMS_TO_TICKS(50))) {
            snprintf(reply, sizeof(reply), "%u\n", roll.value);
        } else {
            snprintf(reply, sizeof(reply), "ERR,CACHE_EMPTY\n");
        }
    } else if (strcasecmp(cmd, "PING") == 0) {
        snprintf(reply, sizeof(reply), "PONG\n");
    } else if (strcasecmp(cmd, "STATS") == 0) {
        reply_stats(reply, sizeof(reply));
    } else if (strcasecmp(cmd, "RESET") == 0) {
        dice_reset_stats(DICE_SRC_TRNG);
        dice_reset_stats(DICE_SRC_PRNG);
        snprintf(reply, sizeof(reply), "OK,RESET\n");
    } else if (strcasecmp(cmd, "CPU") == 0) {
        uint16_t load[CPU_USAGE_CORES];
        cpu_usage_get_last(load);
        snprintf(reply, sizeof(reply), "CPU,%u.%u,%u.%u\n",
                 load[0] / 10, load[0] % 10, load[1] / 10, load[1] % 10);
    } else {
        snprintf(reply, sizeof(reply), "ERR,UNKNOWN,%s\n", cmd);
    }
    send_line(reply);
}

static void serial_link_task(void *arg)
{
    char line[SERIAL_LINE_MAX + 1];
    size_t len = 0;
    bool overflow = false;
    uint8_t buf[64];

    while (1) {
        int n = usb_serial_jtag_read_bytes(buf, sizeof(buf), portMAX_DELAY);
        for (int i = 0; i < n; i++) {
            char c = (char)buf[i];
            if (c == '\n' || c == '\r') {
                // \r\n, \n and \r all end a line; the empty line between \r and \n is ignored
                if (overflow) {
                    send_line("ERR,TOO_LONG\n");
                } else if (len > 0) {
                    line[len] = '\0';
                    handle_command(line);
                }
                len = 0;
                overflow = false;
            } else if (len < SERIAL_LINE_MAX) {
                line[len++] = c;
            } else {
                overflow = true;  // keep discarding until the end of this line
            }
        }
    }
}

void serial_link_start(void)
{
    usb_serial_jtag_driver_config_t cfg = {
        .tx_buffer_size = 512,
        .rx_buffer_size = 256,
    };
    ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&cfg));

    BaseType_t ok = xTaskCreate(serial_link_task, "serial_link", SERIAL_TASK_STACK, NULL,
                                SERIAL_TASK_PRIORITY, NULL);
    configASSERT(ok == pdPASS);
}
