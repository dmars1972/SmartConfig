#ifndef _SV_TYPES_
#define _SV_TYPES_


#define MAX_ROOMS 20
#define MAX_VENTS_PER_ROOM 5
#define VENT_NAME_SIZE 15
#define MAX_SCHEDULE_SET_POINTS 4
#define MAX_HVAC_UNITS 3
#define WEATHER_CHECK_INTERVAL_SECONDS 3600
#define CONFIG_EEPROM_START 0


enum VentState {
  SV_OPEN = 75,
  SV_CLOSED = 0
};

enum HVACMode {
  HEAT,
  COOL,
  FAN,
  NONE
};

enum HVACState {
  HVAC_ON,
  HVAC_OFF
};

struct configStruct {
  char otaIPAddress[4];
  unsigned char roomNumber;
  unsigned char floorNumber;
  bool hasTempSensor;
  char ssid[64];
  char password[64];
  unsigned char autoHeatTemperature;
  unsigned char autoCoolTemperature;
  char zipcode[6];
  char googleMapsAPIKey[64];
  char openWeatherMapAPIKey[64];
};

struct WeatherStruct {
  int weather_id;  
  char description[20];
  char icon[8];
  float outsideTemperature;
  int pressure;
  int humidity;
  float wind_speed;
  float wind_deg;
};

struct EEPROM_Room {
  char roomName[31];
  unsigned char floorNumber;
  unsigned char numVents;
};

struct EEPROM_Vent {
  char ventName[VENT_NAME_SIZE];
};

struct Schedule {
  int startMinute;
  unsigned char temperature;
};

const int ROOM_EEPROM_START = sizeof(configStruct) + 1;
const int VENT_EEPROM_START = ROOM_EEPROM_START + (sizeof(EEPROM_Room) * MAX_ROOMS) + 1;
const int HEAT_SCHEDULE_EEPROM_START = VENT_EEPROM_START + ((VENT_NAME_SIZE * MAX_ROOMS) * MAX_VENTS_PER_ROOM) + 1;
const int COOL_SCHEDULE_EEPROM_START = HEAT_SCHEDULE_EEPROM_START + (sizeof(struct Schedule) * MAX_SCHEDULE_SET_POINTS * 7);

#endif

