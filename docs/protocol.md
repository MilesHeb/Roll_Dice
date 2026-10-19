# Roll_Dice serial protocol

The contract between the ESP32-S3 firmware (`firmware/main/serial_link.c`) and
the LabVIEW VI. Change both sides in the same commit.

## Transport

| Setting | Value |
|---|---|
| Port | Board's native USB port (USB Serial/JTAG), e.g. `COM20` |
| Baud rate | Any (ignored by native USB) |
| Flow control | None. Leave DTR/RTS unasserted: toggling them can reset the ESP32-S3 |
| Termination | Every command and reply ends with `\n` (0x0A). `\r` and `\r\n` also accepted on input |
| Case | Commands are case-insensitive |

The port carries **protocol traffic only**: the firmware's `ESP_LOG` output is
kept off USB (`CONFIG_ESP_CONSOLE_SECONDARY_NONE`, logs go to UART0). So the
line read after sending a command is always that command's reply. Flush the
input buffer once after opening the port, in case the chip's boot messages
are still waiting there.

## The command LabVIEW uses

| Send | Reply | Notes |
|---|---|---|
| `R` | `<roll>`, e.g. `4` | Releases the cached **true random** roll (1-6) and triggers the next one. LabVIEW: VISA Write `R` + Line Feed, one VISA Read, Decimal String To Number |

`R` replies `ERR,CACHE_EMPTY` if the next roll isn't ready within 50 ms (it
normally is ready in microseconds). Decimal String To Number turns any `ERR`
reply into 0, so LabVIEW can treat 0 as "no roll".

## Other commands (testing and the on-board demo)

| Send | Reply | Notes |
|---|---|---|
| `PING` | `PONG` | Connection check |
| `STATS` | `STATS,<T total>,<T1>..<T6>,<P total>,<P1>..<P6>` | Counts from the on-board Core 0 (TRNG) and Core 1 (PRNG) roller tasks shown on the AMOLED. Independent of `R` |
| `RESET` | `OK,RESET` | Zeroes the on-board roller counts |
| `CPU` | `CPU,<core0 %>,<core1 %>` | Per-core load, one decimal, e.g. `CPU,12.3,4.0` |
| anything else | `ERR,UNKNOWN,<command>` | |
| line > 32 chars | `ERR,TOO_LONG` | |

## Example session

```
> R
< 4
> R
< 1
> PING
< PONG
> CPU
< CPU,18.2,21.7
```

Test without LabVIEW: `python tools/serial_test.py COM20`
