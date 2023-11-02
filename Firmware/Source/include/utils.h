
#define pragma once
#define FASTLED_ESP32_FLASH_LOCK 1
#include <FastLED.h>
#include <stdint.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRrecv.h>

#define NUM_LEDS 20
#define WIFI_LED 18
#define IR_LED   19
#define BATT_LED 0
#define MB1_LED  1
#define MB2_LED  4
#define MB3_LED  5
#define MB4_LED  8
#define BOTTOM_LED_1  2
#define BOTTOM_LED_2  3
#define BOTTOM_LED_3  6
#define BOTTOM_LED_4  8
#define RING_LED_1    10
#define RING_LED_2    11
#define RING_LED_3    12
#define RING_LED_4    13
#define RING_LED_5    14
#define RING_LED_6    15
#define RING_LED_7    16
#define RING_LED_8    17
#define BUTTON_LED    9

#define IR_RX_PIN                       43
#define IR_TX_PIN                       1


extern CRGB leds[NUM_LEDS];

struct accl_pos {
    int16_t x,y,z;
};

void InitAudio();
void readMBEEPROM(uint8_t minibadgeSlot, uint8_t data[8]);
void readMBEEPROMID(uint8_t minibadgeSlot, uint8_t serial[16]);
void readMBEEPROMIDspecial(uint8_t minibadgeSlot, uint8_t serial[16]);

void accelGetData(struct accl_pos *out);

extern IRrecv irrecv;
extern IRsend irsend;