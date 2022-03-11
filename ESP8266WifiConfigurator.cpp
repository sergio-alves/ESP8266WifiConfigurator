/*
 * ESP8266WifiConfigurator.cpp - Library to be used on ESP8266 devices with wifi support allowing to 
 * setup sta wifi network, password when connected on the AP. 
 * 
 * When connected to access point with ssid sensor-reader you can connect to http://sensor-reader/configuration/wifi to display the configuration page 
 * or http/192.168.4.1/configuration/wifi if your device cannot resolve the sensor-reader domain name.  
 * 
 * Created by Sergio Alves, January 10, 2022
 * Released into the public domain.
*/

#include "ESP8266WifiConfigurator.h"

/* Initializes the server */
ESP8266WebServer server(80);

/* constructor */
ESP8266WifiConfigurator::ESP8266WifiConfigurator(HardwareSerial * Serial, WifiConfiguration *wifiConfiguration, THandlerFunction persistConfiguration)
{
  ESP8266WifiConfigurator::Serial = Serial; 
  ESP8266WifiConfigurator::wifiConfiguration = wifiConfiguration;
  ESP8266WifiConfigurator::persistConfiguration = persistConfiguration;
}

/* destructor */
ESP8266WifiConfigurator::~ESP8266WifiConfigurator(){
  Serial->println("Destructing WifiConfguratior");
}

/* setups the wifi */
void ESP8266WifiConfigurator::setupWifi() {
  long timeout = 0;
  
  delay(10);
  Serial->println();
  Serial->print("Hi I'm ");
  Serial->println(wifiConfiguration->hostname);
  
  WiFi.mode(WIFI_AP_STA);
  WiFi.hostname(&(wifiConfiguration->hostname[0]));
  WiFi.softAP(&(wifiConfiguration->ap.ssid[0]));
  WiFi.begin(&(wifiConfiguration->sta.ssid[0]), &(wifiConfiguration->sta.password[0]));

  Serial->print("Started WIFI Connection with ");
  Serial->print(wifiConfiguration->sta.ssid);
  Serial->println(" ssid. Starting connecting.");
 
  while (WiFi.status() != WL_CONNECTED && timeout < 10000) {
    timeout+=500;
    delay(500);
    Serial->print(".");
  }

  Serial->println(".");

  IPAddress IP = WiFi.softAPIP();
  Serial->print("AP IP address: ");
  Serial->println(IP);
  Serial->println("WiFi connected");
  Serial->print("IP address: ");
  Serial->println(WiFi.localIP());
}


/* Setups the web server routes */
void ESP8266WifiConfigurator::setupWebServer() {
  
  /* Reboots the module endpoint */
  server.on("/api/reboot-module", HTTP_POST, [this](void) {
    server.send(204, "text/plain", "");
    ESP.restart(); 
  });

  /* serves the jquery library */
  server.on("/static/jquery.min.js", HTTP_GET, [this](void) {
    server.send(200, "application/javascript", jquery_min_js);
  });

  /* serves the small toaster library */
  server.on("/static/toastr.min.js", HTTP_GET, [this](void) {
    server.send(200, "application/javascript", toastr_min_js);
  });

  /* serves the small toaster css */
  server.on("/static/toastr.min.css", HTTP_GET, [this](void) {
    server.send(200, "text/css", toastr_min_css);
  });
  
  /* serves the main wifi configuration and allows to setup both sta, ap and hostname as well */
  server.on("/configuration/wifi", HTTP_GET,  [this](void) {
      Serial->println("-> /configuration/wifi");
      String message = String(index_html);
      message.replace("%STA_SSID%", &(wifiConfiguration->sta.ssid[0]));
      message.replace("%STA_PASSWORD%", &(wifiConfiguration->sta.password[0]));
      message.replace("%AP_SSID%", &(wifiConfiguration->ap.ssid[0]));
      message.replace("%AP_PASSWORD%", &(wifiConfiguration->ap.password[0]));
      message.replace("%HOSTNAME%", &(wifiConfiguration->hostname[0]));
      Serial->print("<- ");
      Serial->println(message);
      server.send(200, "text/html", message);
    }
  );

  /* the endpoint api called to set wifi configuration */
  server.on("/api/setups/wifi", HTTP_POST, [this](void) {
    
    String ssid = "";
    String password = "";
  
    Serial->println("-> /api/setups/wifi");
  
    for (uint8_t i = 0; i < server.args(); i++) {
      if (server.argName(i).equals("staSsid")) {
         strncpy(&(wifiConfiguration->sta.ssid[0]), server.arg(i).c_str(), 16);
      }
      else if (server.argName(i).equals("staPassword")) {
         strncpy(&(wifiConfiguration->sta.password[0]), server.arg(i).c_str(), 16);
      }
      else if (server.argName(i).equals("apSsid")) {
         strncpy(&(wifiConfiguration->ap.ssid[0]), server.arg(i).c_str(), 16);
      }
      else if (server.argName(i).equals("apPassword")) {
         strncpy(&(wifiConfiguration->ap.password[0]), server.arg(i).c_str(), 16);
      }
      else if (server.argName(i).equals("hostname")) {
         strncpy(&(wifiConfiguration->hostname[0]), server.arg(i).c_str(), 32);
      }
    }
    wifiConfiguration->control = WIFI_CONFIGURATION_CONTROL;
      
    ESP8266WifiConfigurator::persistConfiguration();
  
    server.send(204, "text/html", "");
    
  });  
  server.begin();
  ESP8266WifiConfigurator::Serial->println("HTTP server started");
}

/* To be called by arduino setup method */
void ESP8266WifiConfigurator::setup() {
  /* initializes the wifi */
  setupWifi();

  /* initialize the MDNS library in a global way */
  if (!MDNS.begin(&(wifiConfiguration->hostname[0]))) {
    Serial->println("Error setting up MDNS responder! Restarting Module.");
    ESP.restart();
  }
  Serial->println("mDNS responder started");

  /* Setup the webserver */
  setupWebServer();

  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);
}

/* To be called the arduino loop method */
void ESP8266WifiConfigurator::loop() {  
  server.handleClient();
  MDNS.update();
}
