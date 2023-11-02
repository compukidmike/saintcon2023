#include "badge.h"
#include "utils.h"
#include <mutex>
#include <Wire.h>

#define I2C_SDA_PIN                     4
#define I2C_SCL_PIN                     5
#define I2CSWITCH_ADDR                  0x70
#define I2CSWITCH_RST_PIN               6
#define I2CSWITCH_MB1                   0
#define I2CSWITCH_MB2                   1
#define I2CSWITCH_MB3                   2
#define I2CSWITCH_MB4                   3
#define I2CSWITCH_MBEXT                 4
#define I2CSWITCH_ACCEL                 5
#define I2CSWITCH_AUDIO                 6

#define I2CACCEL_ADDR                   0x4C
#define I2CEEPROM_ADDR                  0x50
#define I2CEEPROMSERIAL_ADDR            0x58

std::mutex i2c_mtx;
int i2cswitchposition = 0;

void i2cSwitch(int i2c_channel){
  Wire.beginTransmission(0x70);
  Wire.write(1 << i2c_channel);
  Wire.endTransmission(true);
  i2cswitchposition = i2c_channel;
}

void InitAudio() {
    std::lock_guard<std::mutex> lck(i2c_mtx);
    i2cSwitch(I2CSWITCH_AUDIO);

    Wire.beginTransmission(0b01101100);
    Wire.write(0);
    Wire.write(0);
    Wire.write(0b00000001);
    Wire.endTransmission(true);

    Wire.beginTransmission(0b01101100);
    Wire.write(0);
    Wire.write(0);
    Wire.write(0b00001001);
    Wire.write(0b00001001);
    Wire.write(0b11011000);
    Wire.endTransmission(true);

    Wire.beginTransmission(0b01101100);
    Wire.write(0x13);
    Wire.write(0b00010000);
    Wire.write(0x30);
    Wire.endTransmission(true);
}

void readMBEEPROM(uint8_t minibadgeSlot, uint8_t data[8]){
  std::lock_guard<std::mutex> lck(i2c_mtx);
  if(i2cswitchposition != minibadgeSlot) i2cSwitch(minibadgeSlot);
  int counter = 0;
  Wire.beginTransmission (I2CEEPROM_ADDR);
  if (Wire.endTransmission () == 0) {
    Wire.beginTransmission(I2CEEPROM_ADDR);
    Wire.write(0);
    Wire.endTransmission(false);
    Wire.requestFrom(I2CEEPROM_ADDR, 8);
    while(Wire.available()){
        data[counter] = Wire.read();
        counter ++;
    }
  }
}

void readMBEEPROMID(uint8_t minibadgeSlot, uint8_t serial[16]){
  std::lock_guard<std::mutex> lck(i2c_mtx);  
  if(i2cswitchposition != minibadgeSlot) i2cSwitch(minibadgeSlot);
  int counter = 0;
  Wire.beginTransmission (I2CEEPROMSERIAL_ADDR);
  if (Wire.endTransmission () == 0) {
    Wire.beginTransmission(I2CEEPROMSERIAL_ADDR);
    Wire.write(0x80);
    Wire.endTransmission(false);
    Wire.requestFrom(I2CEEPROMSERIAL_ADDR, 16);
    while(Wire.available()){
        serial[counter] = Wire.read();
        counter ++;
    }
  }
}

//Speical function to read redactd's minibadge
void readMBEEPROMIDspecial(uint8_t minibadgeSlot, uint8_t serial[16]){
  std::lock_guard<std::mutex> lck(i2c_mtx);  
  if(i2cswitchposition != minibadgeSlot) i2cSwitch(minibadgeSlot);
  int counter = 0;
  Wire.beginTransmission (0x13);
  if (Wire.endTransmission () == 0) {
    Wire.beginTransmission(0x13);
    Wire.write(0x80);
    Wire.endTransmission(false);
    Wire.requestFrom(0x13, 16);
    while(Wire.available()){
        serial[counter] = Wire.read();
        counter ++;
    }
  }
}

void accelInit(){
  std::lock_guard<std::mutex> lck(i2c_mtx);
  if(i2cswitchposition != I2CSWITCH_ACCEL) i2cSwitch(I2CSWITCH_ACCEL);
  Wire.beginTransmission(I2CACCEL_ADDR);
  Wire.write(0x07); //Mode Register
  Wire.write(0x01); //WAKE Mode
  Wire.endTransmission(true);
}

void accelGetData(struct accl_pos *out){
  std::lock_guard<std::mutex> lck(i2c_mtx);  
  if(i2cswitchposition != I2CSWITCH_ACCEL) i2cSwitch(I2CSWITCH_ACCEL);
  uint8_t i2cdata[6];
  int counter = 0;
  Wire.beginTransmission(I2CACCEL_ADDR);
  Wire.write(0x0D); //Data Register
  Wire.endTransmission(false);
  Wire.requestFrom(I2CACCEL_ADDR,6);
  //Wire.beginTransmission(I2CACCEL_ADDR);
  //Wire.readBytes(i2cdata,6);
  

  while(Wire.available()){
    i2cdata[counter] = Wire.read();
    counter ++;
  }
  //Wire.endTransmission(true);

  out->x = (i2cdata[1] << 8) | i2cdata[0];
  out->y = (i2cdata[3] << 8) | i2cdata[2];
  out->z = (i2cdata[5] << 8) | i2cdata[4];
}


