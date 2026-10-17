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

The same port also carries the firmware's `ESP_LOG` output (lines starting
with `I (`, `W (`, `E (`). Read lines until one starts with a reply prefix
below and discard the rest. Flush the input buffer after opening the port.

## Commands

| Send | Reply | Notes |
|---|---|---|
| `PING` | `PONG,Dice_Roll` | Connection check |
| `ROLL` | `ROLL,<seq>,<trng>,<prng>` | Releases the cached TRNG + PRNG pair (1-6 each); the cache rerolls immediately. `seq` increases by 1 per pair |
| `STATS` | `STATS,<T total>,<T1>,<T2>,<T3>,<T4>,<T5>,<T6>,<P total>,<P1>,<P2>,<P3>,<P4>,<P5>,<P6>` | Rolling counts from the Core 0 (TRNG) and Core 1 (PRNG) roller tasks |
| `RESET` | `OK,RESET` | Zeroes both rolling counts |
| `CPU` | `CPU,<core0 %>,<core1 %>` | Per-core load, one decimal, e.g. `CPU,12.3,4.0` |
| anything else | `ERR,UNKNOWN,<command>` | |
| line > 32 chars | `ERR,TOO_LONG` | |

`ROLL` can also reply `ERR,CACHE_EMPTY` if the next pair isn't ready within 50 ms.

## Example session

```
> PING
< PONG,Dice_Roll
> ROLL
< ROLL,1,4,2
> ROLL
< ROLL,2,6,1
> STATS
< STATS,1532,262,251,248,259,256,256,1532,255,258,252,254,256,257
> CPU
< CPU,23.4,6.1
```

Test without LabVIEW: `python tools/serial_test.py COM20`
