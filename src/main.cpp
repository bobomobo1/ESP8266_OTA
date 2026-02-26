#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h> // SPIFFS

#include "ota_server.h"
#include "stm32_ota.h"
#include "webpage.h"

// WiFi credentials
const char* ssid = "SM-S916";
const char* password = "superpassword";

void setup() {
  Serial.begin(115200);
  Serial.swap(); // Swaps the default RX/TX pins
  if(!SPIFFS.begin()){
    Serial.println("SPIFFS Mount Failed!");
    return;
  }
  WiFi.begin(ssid,password);
  Serial.print("Connecting");
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected! IP: " + WiFi.localIP().toString());
  ota_server_init();
}

void loop(){
  extern ESP8266WebServer server;
  server.handleClient();
}
