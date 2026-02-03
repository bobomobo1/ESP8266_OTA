/*
* Author: Ethan Aikman
* 
*
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "EnterSSID";
const char* password = "Enter Password";
const char* uploadPage = "<html>"
                         "<body>"
                         "<h2>STM32 Firmware Upload</h2>"
                         "<form method='POST' action='/upload' enctype='multipart/form-data'>"
                         "<input type='file' name='firmware'>"
                         "<input type='submit' value='Upload'>"
                         "</form>"
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
    Serial.printf("Upload Start: %s\n", upload.filename.c_str());

    // Signal STM32 "OTA START"
    Serial1.write(0x55); // Serial1 is tx pin  
  }

  else if (upload.status == UPLOAD_FILE_WRITE) {
    // Stream chunks to STM32 from tx pin this will change later 
    Serial1.write(upload.buf, upload.currentSize);
  }

  else if (upload.status == UPLOAD_FILE_END) {
    Serial.printf("Upload End, Size: %u bytes\n", upload.totalSize);

    // Signal STM32 "OTA END"
    Serial1.write(0xAA);  
  }
}

void setup() {
  Serial.begin(115200);

  // UART to STM32 (TX only is fine)
  Serial1.begin(115200); // GPIO2 (TX)

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

  Serial.print("ESP IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  server.handleClient();
}
