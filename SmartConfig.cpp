#include "SmartConfig.h"

SmartConfig::SmartConfig()
{
  return;
}

void SmartConfig::setConfig(configStruct cs)
{
  EEPROM.begin(4096);

  EEPROM.put(0, cs);

  EEPROM.commit();

  EEPROM.end();

  return;
}

void SmartConfig::setup()
{
  char h[15];
  byte m[6];

  WiFi.macAddress(m);

  sprintf(h, "sv%02x%02x%02x%02x%02x%02x", 
      m[0], 
      m[1], 
      m[2], 
      m[3], 
      m[4], 
      m[5]);

  setup(h);
}

void SmartConfig::setup(const char *hostname)
{
  int pos = 0;
  char encryptedPassword[64];
  spritz_ctx s_ctx;

  Serial.println("Starting configuration setup...");

  EEPROM.begin(4096);
  EEPROM.get(pos, c);

  spritz_setup(&s_ctx, key, sizeof(key));
  spritz_crypt(&s_ctx, (unsigned char *)c.password, sizeof(c.password), (unsigned char *)c.password);  

  Serial.println("Connecting to WiFi");

  WiFi.hostname(hostname);

  WiFi.begin(c.ssid, (char *)c.password);

  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  Serial.println("Connected to WiFi");

  sprintf(OTAIPAddress, "%d.%d.%d.%d", 
      c.otaIPAddress[0], 
      c.otaIPAddress[1], 
      c.otaIPAddress[2], 
      c.otaIPAddress[3]);

  if(strcmp(hostname, "thermostat"))
    return;

  Serial.println("Getting location");
  do {
    getLatLong();
  } while(!locationStatus);

  Serial.println("Done");
}

void SmartConfig::getLatLong()
{
  HTTPClient http;
  char geocodeJSONRequest[64];
  int httpCode;

  Serial.println("Getting Lat/Long...");

  sprintf(geocodeJSONRequest, "http://maps.googleapis.com/maps/api/geocode/json?address=%s", c.zipcode);

  Serial.println(geocodeJSONRequest);

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

    locationStatus = true;
  }

  if(latitude == 0 && longitude == 0) {
    locationStatus = false;
    delay(500);
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

  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
             "Host: " + host + "\r\n" +
             "User-Agent: BuildFailureDetectorESP8266\r\n" +
             "Connection: close\r\n\r\n");


  client.setTimeout(200);

  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r")
      break;
  }

  String line = client.readStringUntil('\r');

Serial.println(line);

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

void SmartConfig::OTAUpdate(const char *currentVersion, const char *deviceType)
{
  static uint32_t lastCheck;
  char OTAURL[128];
  char OTAPATH[26] = "";

  if(lastCheck != 0 && millis() < lastCheck + 60000)
    return;

  lastCheck = millis();

  if(!strcmp(deviceType, "vent"))
    strcpy(OTAPATH, "/svUpdate/svUpdate.php");
  else if (!strcmp(deviceType, "thermostat"))
    strcpy(OTAPATH, "/svUpdate/stUpdate.php");
    
  Serial.print("Checking for updates at ");
  Serial.print(OTAIPAddress);
  Serial.print(OTAPATH);
  Serial.print(" ");
  Serial.println(currentVersion);

  t_httpUpdate_return ret = ESPhttpUpdate.update(OTAIPAddress, 80, OTAPATH, currentVersion);

  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n",  ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
      break;
    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("No updates");
      break;
    case HTTP_UPDATE_OK:
      break;
  }
}
