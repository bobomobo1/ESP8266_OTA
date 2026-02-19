#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h> // SPIFFS

#define MAX_RX_TIMEOUT        3000
#define TX_START_DELIM_1      0x7E
#define TX_START_DELIM_2      0x7E
#define TX_END_DELIM_1        0x7F
#define TX_END_DELIM_2        0x7F
#define TX_DELIMITER_SIZE     2
#define TX_HEADER_SIZE        3 // 2 Bytes(uint16) for packet # and 1 byte(uint8) for packet size
#define TX_CRC_SIZE           4
#define TX_DATA_SIZE          128
#define TX_CHUNK_SIZE         TX_DELIMITER_SIZE + TX_HEADER_SIZE + TX_DATA_SIZE + TX_CRC_SIZE + TX_DELIMITER_SIZE
#define RX_RESPONSE_ACK       0x79

// WiFi credentials
const char* ssid = "28DoonMills";
const char* password = "Summer@2025";

ESP8266WebServer server(80);

// HTML Web UI
String webpage = R"====(
<!DOCTYPE html>
<html>
<head>
<title>STM32 OTA Update</title>
<style>

body {
  font-family: Arial, sans-serif;
  text-align: center;
  margin: 0;
  padding: 20px;
  color: white;

  /* Top 25% white fading to black */
  background-color: black;
  background-image: linear-gradient(to bottom, rgba(255,255,255,0.9), rgba(0,0,0,1) 100%);
  background-repeat: no-repeat;
  background-size: 100% 25vh;
  background-position: top left;
}

/* ================= GLASS CONTAINER ================= */
.glass-card {
  width: 90%;
  max-width: 1000px;
  margin: 30px auto;
  padding: 30px;
  border-radius: 20px;

  background: rgba(20, 20, 20, 0.55);
  backdrop-filter: blur(15px);
  -webkit-backdrop-filter: blur(15px);

  border: 1px solid rgba(255, 255, 255, 0.15);
  box-shadow: 0 0 25px rgba(0,0,0,0.7);
}

/* Header */
h1.overwire {
  display: inline-flex;
  align-items: center;
  font-size: 36px;
  margin-bottom: 10px;
}

img.logo {
  width: 50px;
  height: 50px;
  margin-right: 10px;
}

/* Text colors */
h1.overwire .over { color: black; font-weight: bold; }
h1.overwire .wire { color: skyblue; font-weight: bold; }

h2 {
  margin-top: 10px;
  font-weight: normal;
}

/* File input */
input[type="file"] {
  margin-top: 15px;
  color: white;
  background-color: rgba(0,0,0,0.6);
  border: 1px solid rgba(255,255,255,0.3);
  padding: 8px;
  border-radius: 8px;
}

/* Buttons */
button {
  padding: 10px 25px;
  font-size: 16px;
  margin: 10px 5px;
  cursor: pointer;
  border-radius: 8px;
  border: none;
  background: linear-gradient(45deg, skyblue, #5bbbe0);
  color: black;
  transition: 0.3s;
}

button:hover:enabled {
  transform: scale(1.05);
}

button:disabled {
  background: gray;
  cursor: not-allowed;
}

/* Progress */
progress {
  width: 80%;
  height: 20px;
  margin-top: 20px;
}

/* Table container (glass effect too) */
.table-container {
  margin-top: 25px;
  border-radius: 15px;
  overflow: hidden;

  background: rgba(0,0,0,0.5);
  backdrop-filter: blur(10px);
  border: 1px solid rgba(255,255,255,0.1);
  box-shadow: 0 0 20px rgba(0,0,0,0.6);
}

/* Table */
table {
  width: 100%;
  border-collapse: collapse;
  table-layout: fixed;
  font-size: 14px;
}

/* Column widths */
th:nth-child(1), td:nth-child(1) { width: 10%; }
th:nth-child(2), td:nth-child(2) { width: 20%; }
th:nth-child(3), td:nth-child(3) { width: 30%; }
th:nth-child(4), td:nth-child(4) { width: 15%; }

th, td {
  padding: 10px;
  text-align: center;
  white-space: nowrap;
}

th {
  background-color: rgba(0,0,0,0.7);
  color: skyblue;
}

td {
  background-color: rgba(30,30,30,0.6);
  border-top: 1px solid rgba(255,255,255,0.1);
}

tr:hover td {
  background-color: rgba(60,60,60,0.7);
}

/* Fade-in animation */
.fade-row {
  opacity: 0;
  animation: fadeIn 0.4s forwards;
}

@keyframes fadeIn {
  from { opacity: 0; transform: translateY(5px); }
  to { opacity: 1; transform: translateY(0); }
}

#status {
  margin-top: 20px;
  font-weight: bold;
  color: #00ff88;
}

#percentDisplay {
  margin-top: 10px;
  font-weight: bold;
  color: skyblue;
}

</style>
</head>
<body>

<div class="glass-card">

<h1 class="overwire">
  <img class="logo" src="/OTA_logo.png" alt="Logo" onerror="this.style.display='none'">
  <span class="over">OverThe</span><span class="wire">Wire</span>
</h1>

<h2>STM32 OTA Dashboard</h2>

<input type="file" id="firmware" accept=".bin"><br>
<button onclick="showChunks()">Show Chunks</button>
<button onclick="uploadFirmware()">Upload Firmware</button>
<button id="uploadOTA" onclick="uploadOTAFunction()" disabled>Upload OTA</button>

<progress id="progressBar" value="0" max="100"></progress>
<div id="percentDisplay">0%</div>
<div id="status"></div>

<div class="table-container">
<table>
<thead>
<tr>
<th>Chunk #</th>
<th>Size</th>
<th>Memory_Address-STM32F411RE</th>
<th>Status</th>
</tr>
</thead>
<tbody id="chunkTableBody"></tbody>
</table>
</div>

</div>

<script>
  function showChunks() {
      let file = document.getElementById("firmware").files[0];
    if(!file){ 
          alert("Please select a .bin file"); 
          return; 
      }

    const chunkSize = 1024;
    const baseAddress = 0x08020000;
    let offset = 0;
    let chunkIndex = 0;
    const totalChunks = Math.ceil(file.size / chunkSize);

    document.getElementById("chunkTableBody").innerHTML = "";
    const uploadButton = document.getElementById("uploadOTA");
    uploadButton.disabled = true;

    for(let i=0; i<totalChunks; i++){
        const size = Math.min(chunkSize, file.size - i*chunkSize);
        const memAddress = '0x' + (baseAddress + i*chunkSize).toString(16).toUpperCase().padStart(8,'0');

        let row = document.createElement("tr");
        row.classList.add("fade-row");
        row.style.animationDelay = (i * 0.05) + "s";

        row.innerHTML = `
        <td>${i}</td>
        <td>${size} bytes</td>
        <td>${memAddress}</td>
        <td id="status-${i}" style="color:yellow;">Pending</td>
        `;

        document.getElementById("chunkTableBody").appendChild(row);
  }

    document.getElementById("status").innerHTML =
        `File "${file.name}" selected. Total size: ${file.size} bytes. Preparing chunks...`;

    function processNextChunk(){
        if(offset >= file.size){
            document.getElementById("status").innerHTML =
                `All ${totalChunks} chunks ready for upload`;
            document.getElementById("progressBar").value = 100;
            document.getElementById("percentDisplay").innerText = "100%";
            uploadButton.disabled = false;
            return;
        }

        let cell = document.getElementById("status-"+chunkIndex);
        cell.innerHTML = "Ready";
        cell.style.color = "lime";

        offset += chunkSize;
        let percent = Math.min((offset/file.size)*100, 100);
        document.getElementById("progressBar").value = percent;
        document.getElementById("percentDisplay").innerText = Math.floor(percent) + "%";

        chunkIndex++;
        setTimeout(processNextChunk, 50);
    }

    processNextChunk();
}

function uploadOTAFunction() {
    fetch("/startOTA", { method: "POST" })
    .then(response => response.text())
    .then(data => {
        document.getElementById("status").innerText = data;
    })
    .catch(err => {
        document.getElementById("status").innerText = "OTA failed";
    });
}

function uploadFirmware() {
    let file = document.getElementById("firmware").files[0];
    if (!file) {
        alert("Please select a file first");
        return;
    }

    const formData = new FormData();
    formData.append("firmware", file);

    const xhr = new XMLHttpRequest();
    xhr.open("POST", "/upload", true);

    xhr.upload.onprogress = function(e) {
        if (e.lengthComputable) {
            let percent = (e.loaded / e.total) * 100;
            document.getElementById("progressBar").value = percent;
            document.getElementById("percentDisplay").innerText = Math.floor(percent) + "%";
        }
    };

    xhr.onload = function() {
        if (xhr.status === 200) {
            document.getElementById("status").innerText = "Upload complete!";
        } else {
            document.getElementById("status").innerText = "Upload failed!";
        }
    };

    xhr.send(formData);
}
</script>

</body>
</html>
)====";

// Serve root page
void handleRoot() {
  server.send(200, "text/html", webpage);
}

void handleStartOTA() {
  File firmware = SPIFFS.open("/firmware.bin", "r"); // TODO: Needs to be different then a static name  
  if (!firmware) {
    server.send(500, "text/plain", "Firmware file not found");
    return;
  }
  uint8_t buffer[TX_DATA_SIZE];
  uint16_t packetNumber = 0;
  Serial.println("Starting OTA transfer to STM32...");
  while (firmware.available()) {
    size_t len = firmware.read(buffer, TX_DATA_SIZE);
    // If last packet is smaller than 128 bytes
    if (len < TX_DATA_SIZE) {
        memset(&buffer[len], 0x00, TX_DATA_SIZE - len); // Fill with trailing 0x00
    }
    uint32_t crc = 0xffff; // DUMMY code
    // Clear RX buffer BEFORE sending
    while (Serial.available()) {
      Serial.read();
    }
    /*
      Start of packet code
    */
    // START delimiter
    Serial.write(TX_START_DELIM_1);
    Serial.write(TX_START_DELIM_1);
    // HEADER
    Serial.write((uint8_t*)&packetNumber, sizeof(packetNumber));
    uint8_t dataLen = len;
    Serial.write(dataLen);
    // CHUNK
    Serial.write(buffer, len);
    // CRC
    Serial.write((uint8_t*)&crc, sizeof(crc));
    // END delimiter
    Serial.write(TX_END_DELIM_1);
    Serial.write(TX_END_DELIM_2);
    Serial.flush();   // Ensure transmission finished
    // Wait for 1-byte ACK from STM32
    unsigned long timeout = millis();
    bool ackReceived = false;
    while (millis() - timeout < MAX_RX_TIMEOUT) {
      if (Serial.available()) {
        uint8_t ack = Serial.read();
        if (ack == RX_RESPONSE_ACK) {
          ackReceived = true;
        }
        break;
      }
    }
    if (!ackReceived) {
      firmware.close();
      server.send(500, "text/plain", "ACK timeout or invalid ACK");
      return;
    }
    // Now safely continue to next chunk
  }

  firmware.close();
  server.send(200, "text/plain", "OTA completed successfully");
}


void handleFirmwareUpload() {
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


void handleLogo() {
  File f = SPIFFS.open("/OTA_logo.png", "r");
  if (!f) { 
    server.send(404, "text/plain", "Logo not found"); 
    return; 
  }
  server.streamFile(f, "image/png");
  f.close();
}

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

  server.on("/", handleRoot);
  server.on("/OTA_logo.png", HTTP_GET, handleLogo);
  server.on("/upload", HTTP_POST, []() {
  server.send(200); 
  }, handleFirmwareUpload);
  server.on("/startOTA", HTTP_POST, handleStartOTA);

  server.begin();
}

void loop(){
  server.handleClient();
}
