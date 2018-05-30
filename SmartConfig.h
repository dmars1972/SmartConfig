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

    char OTAIPAddress[16];

    float latitude;
    float longitude;
    int dstOffset;
    int rawOffset;

    bool timezoneStatus;
    bool locationStatus;

    unsigned char key[32] = {'k','a','u','o','k','2','9','u','a','9','$','3','8','7','a','8','9','d','a','8','n','w','?','u','l','s','U','9','3','4','n','o'};

    void getLatLong();
  public:
    SmartConfig();

    // getters
    unsigned char getRoomNumber() { return c.roomNumber; };
    bool getTimezoneStatus() { return timezoneStatus; };
    unsigned char getFloorNumber() { return c.floorNumber; };
    char *getSSID() { return c.ssid; };
    long getTimezoneOffset() { return dstOffset + rawOffset; };
    char getAutoHeatTemperature() { return c.autoHeatTemperature; };
    char getAutoCoolTemperature() { return c.autoCoolTemperature; };
    char *getZipcode() { return c.zipcode; };
    char *getGoogleMapsAPIKey() { return c.googleMapsAPIKey; };
    char *getOpenWeatherMapAPIKey() { return c.openWeatherMapAPIKey; };
    bool hasTempSensor() { return c.hasTempSensor; };

    // setters - because this should only be used in initial setup (or reset) of a device, I didn't go out of my way here

    void setConfig(configStruct);

    // methods
    void queryTimezone(long int);
    void setup();
    void setup(const char *);
    void OTAUpdate(const char *, const char *);
};

#endif
