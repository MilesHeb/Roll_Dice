# Roll_Dice

True vs. pseudo random dice on a LILYGO T-Display-S3 AMOLED 1.91" (ESP32-S3),
displayed with LVGL and streamed to LabVIEW over USB serial.

- **True RNG**: ESP32-S3 hardware RNG with the SAR ADC entropy source enabled
  (`bootloader_random_enable()`), unbiased 1-6 via rejection sampling.
- **Pseudo RNG**: xorshift32 with a fixed seed, as a stand-in for LabVIEW's
  Random Number (0-1). Same sequence after every reset.
- **RTOS**: a TRNG roller task pinned to Core 0 and a PRNG roller task pinned
  to Core 1, each keeping spinlock-protected rolling counts; a one-item queue
  acts as a roll cache released on request; one task owns the serial link.
- **Display**: LVGL 8.3 bar charts of each face's share of rolls, converging
  to 16.7%, plus per-core CPU load.

## Opening in VS Code

Open **`Roll_Dice.code-workspace`** (File > Open Workspace from File...), not
the repo folder. The ESP-IDF extension treats a workspace folder as the
project, so the workspace lists `firmware/` as its own folder (first, so it's
the default for Build / Flash / Monitor / Set Target), plus `labview/` and the
repo root. Opening the repo folder directly makes Set Target and Build fail,
because there is no `CMakeLists.txt` at the root.

## Layout

```
firmware/   ESP-IDF v6.1 project
labview/    LabVIEW project (.lvproj), SubVIs/, Controls/ (typedefs)
tools/      serial_test.py - exercise the serial protocol without LabVIEW
docs/       protocol.md - the serial command contract between both sides
```

## Firmware

Requires ESP-IDF v6.1. From an ESP-IDF terminal:

```
cd firmware
idf.py build
idf.py -p COM20 flash monitor
```

Or open `Roll_Dice.code-workspace` in VS Code and use the ESP-IDF status bar
Build / Flash / Monitor buttons. LVGL 8.3.11 is fetched by the component manager
on the first build (pinned in `dependencies.lock`).

| Module | Role |
|---|---|
| `true_rng.c` | Hardware TRNG, unbiased d6 |
| `pseudo_rng.c` | xorshift32 PRNG, one instance per owner |
| `dice_tasks.c` | Core 0 / Core 1 roller tasks, rolling counts |
| `roll_cache.c` | One-pair cache (depth-1 queue) released on `ROLL` |
| `serial_link.c` | LabVIEW command protocol |
| `cpu_usage.c` | Per-core load from FreeRTOS idle-task run time |
| `dice_ui.c` | LVGL bar charts |
| `display.c`, `rm67162.c` | AMOLED + LVGL driver (ported from LilyGO's lvgl_demo) |

## LabVIEW

Open `labview/Dice_Roll.lvproj` and work through the project window (never
rename or move VIs in Explorer). Conventions for this repo:

- **Separate compiled code from source** is enabled for every VI, so VIs only
  show as changed when actually edited.
- All paths are relative to the project; one LabVIEW version for all files.
- VIs are binary (see `.gitattributes`) and can't be merged: commit small and often.

See `docs/protocol.md` for the serial commands.
