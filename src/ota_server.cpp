#include "ota_server.h"

ESP8266WebServer server(80);

// Serve root page
void handle_root() {
  server.send(200, "text/html", webpage);
}

void handle_firmware_upload() {
  HTTPUpload& upload = server.upload();
  
  if(upload.status == UPLOAD_FILE_START){
    Serial.printf("Upload Start: %s\n", upload.filename.c_str());
    // Create a file in SPIFFS
    File f = SPIFFS.open("/" + upload.filename, "w");
    f.close();
  } else if(upload.status == UPLOAD_FILE_WRITE){
    // Write bytes as they arrive
    File f = SPIFFS.open("/" + upload.filename, "a");
    if(f) {
      f.write(upload.buf, upload.currentSize);
      f.close();
    }
  } else if(upload.status == UPLOAD_FILE_END){
    Serial.printf("Upload Complete: %s, %u bytes\n", upload.filename.c_str(), upload.totalSize);
    server.send(200, "text/plain", "Upload complete");
  }
}


void handle_logo() {
  File f = SPIFFS.open("/OTA_logo.png", "r");
  if (!f) { 
    server.send(404, "text/plain", "Logo not found"); 
    return; 
  }
  server.streamFile(f, "image/png");
  f.close();
}

void ota_server_init(){
  server.on("/", handle_root);
  server.on("/OTA_logo.png", HTTP_GET, handle_logo);
  server.on("/upload", HTTP_POST, []() {
  server.send(200); 
  }, handle_firmware_upload);
  server.on("/startOTA", HTTP_POST, stm32_start_ota);

  server.begin();
}