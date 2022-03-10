// WifiConfiguration.h

#ifndef _WIFICONFIGURATION_h
#define _WIFICONFIGURATION_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include "Html.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>

/* a constant used to now if a configuration was previously saved or not */
#define WIFI_CONFIGURATION_CONTROL 234.567

/* The wifi configuration */
struct WifiConfiguration  {
  struct{
    char ssid[16] = "";
    char password[16] = ""; 
  } sta;
  struct {
    char ssid[16] = "sensor-reader";
    char password[16] = "";
  }ap;
  char hostname[32]="sensor-reader";
  double control = WIFI_CONFIGURATION_CONTROL;
};

/* defines a THandlerFunction pointer ... normally already defined in the ESP8266WebServer lib*/
typedef std::function<void(void)> THandlerFunction;

class WifiConfigurator
{
public:
	WifiConfigurator(HardwareSerial * Serial, WifiConfiguration * wifiConfiguration, THandlerFunction persistConfiguration);
	~WifiConfigurator();
	void setup();
	void loop();
  void setupWifi();
  void setupWebServer();
      
private:
  HardwareSerial* Serial;
  WifiConfiguration * wifiConfiguration;
  THandlerFunction persistConfiguration; // variable to store function pointer type
};


extern WifiConfiguration wifiConfiguration;

#endif
