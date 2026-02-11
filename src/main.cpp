/*
* Author: Ethan Aikman
* 
*
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "28DoonMills";
const char* password = "Summer@2025";
const char* uploadPage =
"<!DOCTYPE html>"
"<html lang='en'>"
"<head>"
"  <meta charset='UTF-8'>"
"  <meta name='viewport' content='width=device-width, initial-scale=1.0'>"
"  <title>STM32 OTA Upload</title>"
"  <style>"
"    body {"
"      margin: 0;"
"      font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;"
"      background: linear-gradient(135deg, #667eea, #764ba2);"
"      height: 100vh;"
"      display: flex;"
"      align-items: center;"
"      justify-content: center;"
"      color: #333;"
"    }"
"    .card {"
"      background: #fff;"
"      border-radius: 12px;"
"      padding: 32px;"
"      width: 320px;"
"      box-shadow: 0 20px 40px rgba(0,0,0,0.2);"
"      text-align: center;"
"    }"
"    h2 {"
"      margin-top: 0;"
"      margin-bottom: 8px;"
"    }"
"    p {"
"      font-size: 0.9rem;"
"      color: #666;"
"      margin-bottom: 24px;"
"    }"
"    input[type=file] {"
"      width: 100%;"
"      margin-bottom: 16px;"
"    }"
"    input[type=submit] {"
"      background: #667eea;"
"      border: none;"
"      color: white;"
"      padding: 10px 16px;"
"      border-radius: 6px;"
"      font-size: 1rem;"
"      cursor: pointer;"
"      width: 100%;"
"      transition: background 0.2s ease;"
"    }"
"    input[type=submit]:hover {"
"      background: #5a67d8;"
"    }"
"    .footer {"
"      margin-top: 16px;"
"      font-size: 0.75rem;"
"      color: #aaa;"
"    }"
"  </style>"
"</head>"
"<body>"
"  <div class='card'>"
"    <h2>STM32 OTA Upload</h2>"
"    <p>Select a .bin firmware file to upload</p>"
"    <form method='POST' action='/upload' enctype='multipart/form-data'>"
"      <input type='file' name='firmware' accept='.bin' required>"
"      <input type='submit' value='Upload Firmware'>"
"    </form>"
"  </div>"
"</body>"
"</html>";


ESP8266WebServer server(80);

File uploadFile;

void handleRoot() {
  server.send(200, "text/html", uploadPage);
}

void handleUpload() {
  HTTPUpload& upload = server.upload();

  if (upload.status == UPLOAD_FILE_START) {
    // Signal STM32 "OTA START"
    Serial.write(0x55); // Serial1 is tx pin  
  }

  else if (upload.status == UPLOAD_FILE_WRITE) {
    // Stream chunks to STM32 from tx pin this will change later 
    Serial.write(upload.buf, upload.currentSize);
  }

  else if (upload.status == UPLOAD_FILE_END) {

    // Signal STM32 "OTA END"
    Serial.write(0xAA);  
  }
}

void setup() {
  Serial.begin(115200);
  Serial.swap();


  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  server.on("/", handleRoot);
  server.on(
    "/upload",
    HTTP_POST,
    []() { server.send(200, "text/plain", "Upload OK"); },
    handleUpload
  );

  server.begin();
}

void loop() {
  server.handleClient();
}
