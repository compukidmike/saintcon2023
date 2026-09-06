#define FASTLED_INTERNAL
#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <sclogo.h>
#include <FS.h>
#include <esp_debug_helpers.h>
#include <HttpsOTAUpdate.h>
#include <esp_https_ota.h>
#include "esp_pm.h"
#include <EEPROM.h>

float firmwareVersion = 1.4;

#include "request.h"
#include "Dungeon.h"
#include "Menu.h"
#include "Keyboard.hpp"
#include "badge.h"
#include "NewCharacter.hpp"
#include "PartyManager.h"
#include "InventoryScreen.hpp"
#include "utils.h"
#include "vending.h"
#include "LevelManager.h"


static const char *updateURL = "https://sc23-api.redactd.net:8443/badge/firmware";

#define BATTERY_SENSE_PIN               2
#define MINIBADGE_CLK_CTRL_H_PIN        10
#define MINIBADGE_CLK_CTRL_L_PIN        9
#define ACCEL_INT_PIN                   3

#define I2S_SDI_PIN                     44
#define I2S_LRCK_PIN                    41
#define I2S_SCK_PIN                     42

#define LCD_DC_PIN                      37
#define LCD_CS_PIN                      38
#define LCD_WR_PIN                      36
#define LCD_RST_PIN                     35
#define LCD_D0_PIN                      45
#define LCD_D1_PIN                      48
#define LCD_D2_PIN                      47
#define LCD_D3_PIN                      21
#define LCD_D4_PIN                      14
#define LCD_D5_PIN                      13
#define LCD_D6_PIN                      12
#define LCD_D7_PIN                      11
#define LCD_BACKLIGHT_PIN               46

CRGB leds[NUM_LEDS];

unsigned long lastButtonPress = 0;
int screenDimDelay = 10000;
int screenOffDelay = 15000;

static HttpsOTAStatus_t otastatus;

Arduino_DataBus *bus = new Arduino_ESP32LCD8(37 /* DC */, 38 /* CS */, 36 /* WR */, -1 /* RD */,
    45 /* D0 */, 48 /* D1 */, 47 /* D2 */, 21 /* D3 */, 14 /* D4 */, 13 /* D5 */, 12 /* D6 */, 11 /* D7 */);

Arduino_G *outputdisplay = new Arduino_ST7789(bus, 35,3,true);
Arduino_Canvas *gfx = new Arduino_Canvas(320,240,outputdisplay);



TaskHandle_t wifiTask;
int n = 0;

WiFiClient wifi;


const char* root_ca= \
"-----BEGIN CERTIFICATE-----\n" \
"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\n" \
"-----END CERTIFICATE-----\n";



struct tm timeinfo;

Keyboard keyboard;


// put function declarations here:
void accelInit();
void accelGetData();
void wifiTaskCode(void * pvParameters );
void setLCDBacklight(uint8_t level);
void hardwareTest();
void periodicEvents(void * pvParameters);

void IRAM_ATTR KEYBOARD_ISR() {
    keyboard.Update();
    lastButtonPress = millis();
}

void HttpEvent(HttpEvent_t *event)
{
    switch(event->event_id) {
        case HTTP_EVENT_ERROR:
            puts("Http Event Error");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            puts("Http Event On Connected");
            break;
        case HTTP_EVENT_HEADER_SENT:
            //puts("Http Event Header Sent");
            //puts(event->header_key);
            //puts(event->header_value);
            break;
        case HTTP_EVENT_ON_HEADER:
            //puts("Http Event On Header");
            break;
        case HTTP_EVENT_ON_DATA:
            //puts("Http Event On Data");
            break;
        case HTTP_EVENT_ON_FINISH:
            puts("Http Event On Finish");
            break;
        case HTTP_EVENT_DISCONNECTED:
            puts("Http Event Disconnected");
            break;
    }
}

void heap_caps_alloc_failed_hook(size_t requested_size, uint32_t caps, const char *function_name)
{
  printf("%s was called but failed to allocate %d bytes with 0x%X capabilities. \n",function_name, requested_size, caps);
  heap_caps_print_heap_info(MALLOC_CAP_DMA);
  esp_backtrace_print(20);
}

void setup() {
  // put your setup code here, to run once:

  puts("Saintcon 2023 Badge");
  heap_caps_malloc_extmem_enable(512);
  esp_err_t error = heap_caps_register_failed_alloc_callback(heap_caps_alloc_failed_hook);

  pinMode(6, OUTPUT);
  digitalWrite(6, HIGH);

  pinMode(MINIBADGE_CLK_CTRL_H_PIN, OUTPUT);
  pinMode(MINIBADGE_CLK_CTRL_L_PIN, OUTPUT);

  keyboard.Begin();
  LoadBadgeId();

  gfx->begin(30000000);
  
  gfx->draw16bitRGBBitmap(0,0,(uint16_t*)sclogo,320,240);
  gfx->flush();

  setLCDBacklight(128);

  FastLED.addLeds<NEOPIXEL,7>(leds, NUM_LEDS);
  FastLED.setBrightness(25);

  keyboard.Update();
    lastButtonPress = millis();

  if(keyboard.IsKeyDown(6)) {
    keyboard.ClearEvents();
    while (!keyboard.KeyPressEvent(KEY_A))
      hardwareTest();
  }

  for(int x=0; x<8; x++){
    leds[x+10] = CRGB::White;
    FastLED.show();
    delay(50);
  }

  for(int x=0; x<8; x++){
    leds[x+10] = CRGB::Black;
    FastLED.show();
    delay(50);
  }

  EEPROM.begin(64);

  Wire.setPins(4,5);
  Wire.setTimeOut(100);
  Wire.begin();

  irsend.begin();

  accelInit();

  TaskHandle_t periodicTaskHandle = NULL;

  xTaskCreateUniversal(periodicEvents, "periodic", 8192, NULL, 1, &periodicTaskHandle, ARDUINO_RUNNING_CORE);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.setAutoReconnect(true);
  
  if (!WifiEnable()) {
    if (!ConfigWifi()) {
      ESP.restart();
    }
  }

  InitAudio();

  delay(500);

  pinMode(IR_RX_PIN, INPUT_PULLUP);

  HttpsOTA.onHttpEvent(HttpEvent);

  if(RequestFirmwareVersion()){
    puts("Starting OTA Update");
    gfx->fillScreen(0xf800);
    gfx->setTextSize(3);
    gfx->setTextColor(0xFFFF);
    gfx->setCursor(10, 10);
    gfx->print("Updating Firmware");
    gfx->setTextSize(2);
    gfx->setCursor(10, 50);
    gfx->println("This can take a few");
    gfx->println(" minutes");
    gfx->setCursor(10, 120);
    gfx->print("Please wait");
    gfx->flush();

    HttpsOTA.begin(updateURL, root_ca);
    extern esp_https_ota_handle_t https_ota_handle;
    int frameCounter = 0;
    while (1){
      otastatus = HttpsOTA.status();
      if(otastatus == HTTPS_OTA_SUCCESS) { 
          puts("Firmware written successfully");
          gfx->fillRect(10,120,310,50,0xf800);
          gfx->setCursor(10, 120);
          gfx->println("Update Successful!");
          gfx->print("Rebooting...");
          gfx->flush();
          delay(3000);
          ESP.restart();
          break;
      } else if(otastatus == HTTPS_OTA_FAIL) { 
          puts("Firmware Upgrade Fail");
          gfx->fillRect(10,120,310,50,0xf800);
          gfx->setCursor(10, 120);
          gfx->println("Update Failed :(");
          gfx->println("Please reboot to try again");
          gfx->flush();
          while(1);
          break;
      }
      delay(1000);

      gfx->fillRect(10,140,310,50,0xf800);
      gfx->setCursor(10, 140);
      for(int x=0; x<frameCounter; x++){
        gfx->print(".");
      }
      frameCounter ++;
      if(frameCounter > 16) frameCounter = 0;
      gfx->flush();
      puts("Downloading Update...");
    }
  }
  
  RequestLoadBadge();
}

void loop() {

  Menu mmenu;
  mmenu.SetTitle("Main Menu");
  mmenu.AllowEscape(false);
  mmenu.AddOption("Incidents");
  mmenu.AddOption("Party");
  mmenu.AddOption("Inventory");
  mmenu.AddOption("Vending machine");
  mmenu.AddOption("Edit Character");
  mmenu.AddOption("Level-up");
  mmenu.AddOption("Wifi");

  String pick = mmenu.Run();
  if (pick.equals("Incidents")) {
    int did = DungeonSelect();
    if (did >=0) 
      Dungeon(did).Run();
  }
  else if (pick.equals("Edit Character")) {
    UpdateCharacter();
  }
  else if (pick.equals("Inventory")) {
    InventoryScreen().Run();
  }
  else if (pick.equals("Party")) {
    ShowPartyManager();
  }
  else if (pick.equals("Vending machine")) {
    ShowVendingScreen();
  }
  else if (pick.equals("Wifi")) {
    ConfigWifi();
  }
  else if (pick.equals("Level-up")) {
      ShowLevelMenu();
  }

  
}

// put function definitions here:

//This is for the things that need to run at regular intervals,
//independent of what's going on with the rest of the badge
void periodicEvents(void * pvParameters){ 
  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount();

  while (1){ 
    ScanMinibadges();

    //Update Minibadge CLK pin
    if(digitalRead(MINIBADGE_CLK_CTRL_H_PIN)){
      digitalWrite(MINIBADGE_CLK_CTRL_H_PIN, LOW);
      digitalWrite(MINIBADGE_CLK_CTRL_L_PIN, HIGH);
    } else {
      digitalWrite(MINIBADGE_CLK_CTRL_H_PIN, HIGH);
      digitalWrite(MINIBADGE_CLK_CTRL_L_PIN, LOW);
    }

    //Wait for the next 1 second interval
    xTaskDelayUntil(&xLastWakeTime, 1000);
  }
}



void wifiTaskCode(void * pvParameters ){
  for(;;){
    n = WiFi.scanNetworks();
    delay(5000);
  }
}

void setLCDBacklight(uint8_t level){
  analogWrite(LCD_BACKLIGHT_PIN, level); //0-255
}

void hardwareTest(){
  extern String badge_id;
  static unsigned long lastTime = 0;
  static unsigned long drawTime = 0;
  static unsigned long blitTime = 0;
  unsigned long startTime = millis();
  gfx->setTextSize(1);
  gfx->fillScreen(BLACK);
  gfx->setCursor(10, 10);
  gfx->setTextColor(RED);
  gfx->print("SAINTCON 2023 Hardware Test");
  gfx->print("        Battery: ");
  gfx->print((((double)analogRead(2)*33)/40960),3);
  gfx->println("V");
  gfx->println(" ");
  gfx->print("Firmware Version: ");
  gfx->println(firmwareVersion);
  gfx->println(" ");
  //gfx->print("Badge ID: ");
  //gfx->println(badge_id);
  gfx->println(" ");
  gfx->print("FPS: ");
  gfx->println(1000/(startTime - lastTime));
  lastTime = startTime;

  gfx->print("Free memory: ");
  gfx->println(esp_get_free_heap_size());

  gfx->print("Free internal memory: ");
  gfx->println(esp_get_free_internal_heap_size());

  gfx->print("Minimum free memory: ");
  gfx->println(esp_get_minimum_free_heap_size());


  struct accl_pos loc;

  accelGetData(&loc);
  gfx->println();
  gfx->print("Accel X: ");
  gfx->println(loc.x,10);
  gfx->print("Accel Y: ");
  gfx->println(loc.y,10);
  gfx->print("Accel Z: ");
  gfx->println(loc.z,10);
  gfx->println();
  gfx->drawFastHLine(200,50,100,0xFFFF);
  gfx->drawFastHLine(200,150,100,0xFFFF);
  gfx->drawFastVLine(200,50,100,0xFFFF);
  gfx->drawFastVLine(300,50,100,0xFFFF);
  gfx->drawPixel((loc.y/655)+250,(loc.x/655)+100,0xFFFF);
  gfx->drawPixel(310,(loc.z/655)+100,0xFFFF);

  char eepromBytes[9] = {0};
  readMBEEPROM(0,(uint8_t*)eepromBytes);
  //uint32_t eepromid = readMBEEPROM(1);
  gfx->print("Minibadge 1: ");
  gfx->println((eepromBytes));
  if(eepromBytes[0] > 0){
    leds[1] = CRGB::White;
  } else {
    leds[1] = 0x140000;
  }
  for(int x=0; x<8; x++) eepromBytes[x] = 0;
  readMBEEPROM(1,(uint8_t*)eepromBytes);
  //uint32_t eepromid = readMBEEPROM(1);
  gfx->print("Minibadge 2: ");
  gfx->println((eepromBytes));
  if(eepromBytes[0] > 0){
    leds[4] = CRGB::White;
  } else {
    leds[4] = 0x140000;
  }
  for(int x=0; x<8; x++) eepromBytes[x] = 0;
  readMBEEPROM(2,(uint8_t*)eepromBytes);
  //uint32_t eepromid = readMBEEPROM(1);
  gfx->print("Minibadge 3: ");
  gfx->println((eepromBytes));
  if(eepromBytes[0] > 0){
    leds[5] = CRGB::White;
  } else {
    leds[5] = 0x140000;
  }
  for(int x=0; x<8; x++) eepromBytes[x] = 0;
  readMBEEPROM(3,(uint8_t*)eepromBytes);
  //uint32_t eepromid = readMBEEPROM(1);
  gfx->print("Minibadge 4: ");
  gfx->println((eepromBytes));
  if(eepromBytes[0] > 0){
    leds[8] = CRGB::White;
  } else {
    leds[8] = 0x140000;
  }


  if(digitalRead(IR_RX_PIN)){
    leds[IR_LED] = CRGB::Black;
  } else {
    leds[IR_LED] = CRGB::White;
  }

  FastLED.show();
  
  drawTime = millis();
  gfx->print("Draw Time: ");
  gfx->println(drawTime - startTime);

  gfx->print("Blit Time: ");
  gfx->println(blitTime);

  gfx->println();
  gfx->println();

  gfx->print("WiFi Status:");
      switch(WiFi.status()){
        case 0:
          gfx->println("Idle");
          break;
        case 1:
          gfx->println("No SSID Avail");
          leds[WIFI_LED] = CRGB::Red;
          break;
        case 2:
          gfx->println("Scan Complete");
          break;
        case 3:
          gfx->println("Connected");
          gfx->println(WiFi.localIP());
          if(!leds[WIFI_LED] == CRGB::Green) leds[WIFI_LED] = CRGB::Yellow;
          break;
        case 4:
          gfx->println("Connect Failed");
          leds[WIFI_LED] = CRGB::Red;
          break;
        case 5:
          gfx->println("Connection Lost");
          leds[WIFI_LED] = CRGB::Red;
          break;
        case 6:
          gfx->println("Disconnected");
          leds[WIFI_LED] = CRGB::Red;
          break;
      }

  gfx->flush();

  blitTime = millis() - drawTime;

}