# Flash attract-mode firmware

Idle on the main menu for **15 seconds** to start the attract / demo loop. Any button returns to the menu. Original badge gameplay is still there.

## Easiest: browser flash (Chrome / Edge)

1. Plug the badge in over USB.
2. Open [Adafruit WebSerial ESPTool](https://adafruit.github.io/Adafruit_WebSerial_ESPTool/).
3. Connect to the badge serial port.
4. Erase if you like (optional), then flash **`saintcon2023-attract-full.bin`** at offset **`0x0`**.

## Easy: one-liner with esptool

```bash
pip install esptool
```

**Full image** (bootloader + partitions + app) — recommended:

```bash
esptool.py --chip esp32s3 write_flash 0x0 saintcon2023-attract-full.bin
```

**App only** (badge already has a working bootloader):

```bash
esptool.py --chip esp32s3 write_flash 0x10000 firmware.bin
```

If the port is flaky at high speed, add `--baud 115200`.

### Full wipe / separate bins

```bash
esptool.py --chip esp32s3 write_flash \
  0x0 bootloader.bin \
  0x8000 partitions.bin \
  0xe000 boot_app0.bin \
  0x10000 firmware.bin
```

## Files in this folder

| File | Purpose |
|------|---------|
| `saintcon2023-attract-full.bin` | Merged image — flash at `0x0` |
| `firmware.bin` | App only — flash at `0x10000` |
| `bootloader.bin` / `partitions.bin` / `boot_app0.bin` | Needed for a full wipe |

Build from source lives under `Firmware/Source/` (PlatformIO).
