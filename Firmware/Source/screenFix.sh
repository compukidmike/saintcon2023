# This scripts will fix the GLX issue from PlatformIO as stated in ../README.md 

sed -i '/Arduino_ESP32LCD8::writePixels(uint16_t \*data, uint32_t len)/,/while (len--)/ {
    /Arduino_ESP32LCD8::writePixels(uint16_t \*data, uint32_t len)/! {
        /while (len--)/! s/^/\/\/ /
    }
}' ".pio/libdeps/esp32-s3-devkitc-1/GFX Library for Arduino/src/databus/Arduino_ESP32LCD8.cpp"

sed -i '/Arduino_ESP32LCD8::writePixels(uint16_t \*data, uint32_t len)/ {
    n;
    s/^\/\/ //;
}' ".pio/libdeps/esp32-s3-devkitc-1/GFX Library for Arduino/src/databus/Arduino_ESP32LCD8.cpp"
