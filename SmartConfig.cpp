#include "SmartConfig.h"

SmartConfig::SmartConfig()
{
  return;
}

void SmartConfig::setup()
{
  int pos = 0;
  char encryptedPassword[64];
  spritz_ctx s_ctx;

  Serial.println("Starting configuration setup...");

  EEPROM.begin(4096);
  EEPROM.get(pos, c);

/*
  EEPROM.get(pos, otaIPAddress);
  pos += sizeof(otaIPAddress);
  EEPROM.get(pos, roomNumber);
  pos += sizeof(roomNumber);
  EEPROM.get(pos, floorNumber);
  pos += sizeof(floorNumber);
  EEPROM.get(pos, hasTempSensor);
  pos += sizeof(hasTempSensor);
  EEPROM.get(pos, ssid);
  pos += sizeof(ssid);
  EEPROM.get(pos, encryptedPassword);
  pos += sizeof(encryptedPassword);
  EEPROM.get(pos, autoHeatTemperature);
  pos += sizeof(autoHeatTemperature);
  EEPROM.get(pos, autoCoolTemperature);
  pos += sizeof(autoCoolTemperature);
  
  EEPROM.get(pos, zipcode);
  pos += sizeof(zipcode);
  EEPROM.get(pos, googleMapsAPIKey);
  pos += sizeof(googleMapsAPIKey);
  EEPROM.get(pos, openWeatherMapAPIKey);
*/

  spritz_setup(&s_ctx, key, sizeof(key));
  spritz_crypt(&s_ctx, (unsigned char *)c.password, sizeof(c.password), (unsigned char *)c.password);  

  WiFi.hostname("thermostat");

  WiFi.begin(c.ssid, (char *)c.password);

  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  getLatLong();
}

void SmartConfig::getLatLong()
{
  HTTPClient http;
  char geocodeJSONRequest[64];
  int httpCode;

  Serial.println("Getting Lat/Long...");

  sprintf(geocodeJSONRequest, "http://maps.googleapis.com/maps/api/geocode/json?address=%s", c.zipcode);

  http.begin(geocodeJSONRequest);
  httpCode = http.GET();
  if(httpCode > 0) {
    const size_t bufferSize = 3*JSON_ARRAY_SIZE(1) + 4*JSON_ARRAY_SIZE(2) + JSON_ARRAY_SIZE(4) + JSON_ARRAY_SIZE(5) + 8*JSON_OBJECT_SIZE(2) + 5*JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(6) + 860;
    DynamicJsonBuffer jsonBuffer(bufferSize);
    JsonObject& root = jsonBuffer.parseObject(http.getString());
    JsonObject& results0 = root["results"][0];

    JsonObject& results0_geometry = results0["geometry"];

    latitude = results0_geometry["bounds"]["northeast"]["lat"]; // 33.184899
    longitude = results0_geometry["bounds"]["northeast"]["lng"]; // -97.060992

  }

  http.end();

  return;
}

void SmartConfig::queryTimezone(long int timestamp)
{
  WiFiClientSecure client;
  const char *host = "maps.google.com";
  const char *fingerprint = "C4 05 BD 13 00 93 28 52 4B 70 F1 44 4A D2 0D 7A 07 7C 1D 4A";
  char url[128];
  char timezoneJSONRequest[256];
  int httpCode;
  const size_t bufferSize = JSON_OBJECT_SIZE(5) + 120;
  DynamicJsonBuffer jsonBuffer(bufferSize);
  const char* json = "{\"dstOffset\":0,\"rawOffset\":-21600,\"status\":\"OK\",\"timeZoneId\":\"America/Chicago\",\"timeZoneName\":\"Central Standard Time\"}";

  if(timezoneStatus == true)
    return;

  Serial.println("Getting timezone offset...");

  sprintf(url, "/maps/api/timezone/json?location=%3.6f,%3.6f&timestamp=%d&key=%s", latitude, longitude, timestamp, c.googleMapsAPIKey);

  if(!client.connect(host, 443)) {
    Serial.println("Connect to maps.google.com failed");
    return;
  }

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
             "Host: " + host + "\r\n" +
             "User-Agent: BuildFailureDetectorESP8266\r\n" +
             "Connection: close\r\n\r\n");


  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r")
      break;
  }

  String line = client.readStringUntil('\r');

//Serial.println(line);

  if (line.indexOf("\"status\" : \"OK\"") != -1) {
    JsonObject& root = jsonBuffer.parseObject(line);

    dstOffset = atol(root["dstOffset"]); // 0
    rawOffset = atol(root["rawOffset"]); // -21600

    timezoneStatus = true;
  } else {
    timezoneStatus = false;
  }

  client.stop();

  Serial.print("DST offset: ");
  Serial.println(dstOffset);
  Serial.print("Raw offset: ");
  Serial.println(rawOffset);
  return;
}
