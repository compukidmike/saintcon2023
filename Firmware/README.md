# Firmware

The badge code is written in C++ using PlatformIO in VSCode. Compiling from source is an involved process and I can guarantee I've missed some steps here.

Install VSCode. 
Add the PlatformIO extension.
Open the Saintcon2023 folder in VSCode. PlatformIO should automatically setup the project for you and download the required libraries, etc.
You'll need to make a few changes for it to compile:
- Open this file: `.pio\libdeps\esp32-s3-devkitc-1\GFX Library for Arduino\src\databus\Arduino_ESP32LCD8.cpp`
- Comment out most of the function `void Arduino_ESP32LCD8::writePixels`, leaving just this at the end:
`while (len--)
  {
    _data16.value = *data++;
    LCD_CAM.lcd_cmd_val.lcd_cmd_value = _data16.msb;
    WAIT_LCD_NOT_BUSY;
    LCD_CAM.lcd_user.val = LCD_CAM_LCD_CMD | LCD_CAM_LCD_UPDATE_REG | LCD_CAM_LCD_START;
    LCD_CAM.lcd_cmd_val.lcd_cmd_value = _data16.lsb;
    WAIT_LCD_NOT_BUSY;
    LCD_CAM.lcd_user.val = LCD_CAM_LCD_CMD | LCD_CAM_LCD_UPDATE_REG | LCD_CAM_LCD_START;
  }`
- Open this file: `.pio\libdeps\esp32-s3-devkitc-1\WebServer_ESP32_W6100\src\WebServer_ESP32_W6100.hpp`
- Comment out this line: `#include <WebServer.h> // Introduce corresponding libraries`

I'm sure there are other things. Please let me know or submit a pull request.