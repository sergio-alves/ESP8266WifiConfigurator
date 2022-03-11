#include <ESP8266WifiConfigurator.h>

#define EEPROM_SIZE 512

WifiConfiguration wifiConfiguration;

WifiConfigurator wifiConfigurator(&Serial, &wifiConfiguration, []() {
  EEPROM.put(0, wifiConfiguration);
  EEPROM.commit();
 });

void persistConfiguration() {
  EEPROM.put(0, wifiConfiguration);
  EEPROM.commit();
}

void getConfiguration() {
  EEPROM.get(0, wifiConfiguration);
  
  if(wifiConfiguration.control != WIFI_CONFIGURATION_CONTROL) {
    memset(&wifiConfiguration,'\0', sizeof(WifiConfiguration));
    
    strncpy(&(wifiConfiguration.sta.ssid[0]), "local_ssid", 16);
    strncpy(&(wifiConfiguration.sta.password[0]), "local_password", 16);
    strncpy(&(wifiConfiguration.ap.ssid[0]), "sensor-reader", 16);
    strncpy(&(wifiConfiguration.ap.password[0]), "", 16);
    strncpy(&(wifiConfiguration.hostname[0]), "sensor-reader", 32);
    wifiConfiguration.control=WIFI_CONFIGURATION_CONTROL;
        
    EEPROM.put(0, wifiConfiguration);
    EEPROM.commit();      
  }

}

void setup()
{
  // Starts the begin
  Serial.begin(115200);
  delay(1000);

  // Initialize the EEPROM simulation for the app
  EEPROM.begin(EEPROM_SIZE);
  getConfiguration();
	
	wifiConfigurator.setup();
}

void loop()
{
	wifiConfigurator.loop();
}
