/*
 * This class does not itself manage the the WiFi on the ESP-01. Instead, it handles
 * the management of network connectivity, specifically, it simplifies the process of
 * connecting to available networks without having to hardcode credentials into the
 * ESP-01.
 * 
 * While the class itself is new, much of the functionality and large bits of code have
 * been drawn from https://how2electronics.com/esp8266-manual-wifi-configuration-without-hard-code-with-eeprom/#ESP8266_Manual_Wifi_Configuration_with_EEPROM
 */

#ifndef WiFiManager_H
#define WiFiManager_H

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

#define WIFI_CONNECTED 0
#define WIFI_DISCONNECTED 1
#define WIFI_CONFIG 2

class WiFiManager {

private:
  int statusCode;
  String st;
  String content;
  String esid;
  int led;
  String AP_SSID;
  String AP_PWD;
  String hostname;
  int ON_SIGNAL;
  int OFF_SIGNAL;
  ESP8266WebServer server;
  
  bool testWiFi();
  void initialize();
  void launchWeb();
  void rescan();
  void setupAP();
  void createWebServer();
  void setDefaultAP();
  

public:
  WiFiManager();
  ~WiFiManager();

  void setAPCredentials(String, String);
  void setHostname(String);
  void setAPIP(IPAddress);
  void start();
  int state();
  int setLed(int, int);
};
    
#endif
