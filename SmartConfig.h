#ifndef _SMART_CONFIG
#define _SMART_CONFIG

#include "Arduino.h"

#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <SpritzCipher.h>
#include <ESP8266httpUpdate.h>
#include <ArduinoJson.h>
#include "types.h"

class SmartConfig {
  private:
    configStruct c;

    float latitude;
    float longitude;
    int dstOffset;
    int rawOffset;

    bool timezoneStatus;

    unsigned char key[32] = {'k','a','u','o','k','2','9','u','a','9','$','3','8','7','a','8','9','d','a','8','n','w','?','u','l','s','U','9','3','4','n','o'};

    void getLatLong();
  public:
    SmartConfig();

    void queryTimezone(long int);
    bool getTimezoneStatus() { return timezoneStatus; };
    void setup();
    unsigned char getFloorNumber() { return c.floorNumber; };
    char *getSSID() { return c.ssid; };
    long getTimezoneOffset() { return dstOffset + rawOffset; };

    char getAutoHeatTemperature() { return c.autoHeatTemperature; };
    char getAutoCoolTemperature() { return c.autoCoolTemperature; };
    char *getZipcode() { return c.zipcode; };
    char *getOpenWeatherMapAPIKey() { return c.openWeatherMapAPIKey; };
};

#endif
