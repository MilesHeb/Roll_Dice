"""Quick check of the Dice_Roll serial protocol before connecting LabVIEW.

Usage (close VS Code's ESP-IDF monitor first - only one program can open the port):
    python tools/serial_test.py COM20
"""
import sys
import time

import serial  # pyserial (included in the ESP-IDF Python environment)

PORT = sys.argv[1] if len(sys.argv) > 1 else "COM20"

# Replies start with one of these; anything else on the port is ESP_LOG output
REPLY_PREFIXES = ("PONG", "ROLL", "STATS", "OK", "CPU", "ERR")

# Leave DTR/RTS low: toggling them on the ESP32-S3 USB port can reset the chip
with serial.Serial() as s:
    s.port = PORT
    s.baudrate = 115200  # ignored by USB Serial/JTAG, any value works
    s.timeout = 1
    s.dtr = False
    s.rts = False
    s.open()
    time.sleep(0.2)
    s.reset_input_buffer()

    def ask(cmd):
        s.write((cmd + "\n").encode())
        deadline = time.time() + 2
        while time.time() < deadline:
            line = s.readline().decode(errors="replace").strip()
            if line.startswith(REPLY_PREFIXES):
                print(f"{cmd:<6} -> {line}")
                return line
            if line:
                print(f"         (log) {line}")
        print(f"{cmd:<6} -> no reply")
        return None

    ask("PING")
    for _ in range(3):
        ask("ROLL")
    ask("STATS")
    ask("CPU")
    ask("BOGUS")
