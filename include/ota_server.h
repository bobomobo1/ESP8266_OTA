#ifndef OTA_SERVER_H
#define OTA_SERVER_H

#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <FS.h>

#include "ota_server.h"
#include "stm32_ota.h"
#include "webpage.h"

void handle_firmware_upload();
void handle_root();
void handle_logo();
void ota_server_init();

#endif