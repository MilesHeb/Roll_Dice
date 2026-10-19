"""Quick check of the Dice_Roll serial protocol before connecting LabVIEW.

Sends "R" 60 times and prints the rolls plus a text histogram - the same job
the LabVIEW VI does - then checks the other commands.

Usage (close VS Code's ESP-IDF monitor and any LabVIEW VI using the port first):
    python tools/serial_test.py COM20
"""
import sys
import time

import serial  # pyserial (included in the ESP-IDF Python environment)

PORT = sys.argv[1] if len(sys.argv) > 1 else "COM20"
ROLLS = 60

# Leave DTR/RTS low: toggling them on the ESP32-S3 USB port can reset the chip
with serial.Serial() as s:
    s.port = PORT
    s.baudrate = 115200  # ignored by USB Serial/JTAG, any value works
    s.timeout = 1
    s.dtr = False
    s.rts = False
    s.open()
    time.sleep(0.2)
    s.reset_input_buffer()  # drop any boot messages

    def ask(cmd):
        s.write((cmd + "\n").encode())
        return s.readline().decode(errors="replace").strip() or None

    print(f"PING   -> {ask('PING')}")

    counts = [0] * 6
    rolls = []
    for _ in range(ROLLS):
        reply = ask("R")
        if reply is None or not reply.isdigit() or not 1 <= int(reply) <= 6:
            print(f"R      -> unexpected reply: {reply!r}")
            sys.exit(1)
        rolls.append(int(reply))
        counts[int(reply) - 1] += 1
    print(f"R x{ROLLS} -> {' '.join(map(str, rolls))}")
    for face, n in enumerate(counts, start=1):
        print(f"  {face}: {'#' * n} {n}")

    print(f"STATS  -> {ask('STATS')}")
    print(f"CPU    -> {ask('CPU')}")
    print(f"BOGUS  -> {ask('BOGUS')}")
