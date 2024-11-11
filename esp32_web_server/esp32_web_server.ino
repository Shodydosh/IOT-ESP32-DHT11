#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <esp_now.h>
#include <IRremote.hpp>

#define DHTPIN 14
#define DHTTYPE DHT11
#define IR_SEND_PIN 18
const char* ap_ssid = "ESP32_GIAMSAT_AP";
const char* ap_password = "12345678";

uint16_t onSignal[199] = {4488, 4396,  562, 1670,  534, 544,  586, 1650,  536, 1670,  536, 544,  566, 540,  568, 1642,  556, 546,  590, 514,  586, 1646,  536, 544,  574, 532,  562, 1670,  536, 1670,  534, 544,  562, 1654,  584, 1618,  558, 544,  586, 1652,  534, 1670,  536, 1646,  558, 1670,  536, 1670,  536, 1672,  562, 516,  582, 1630,  558, 544,  586, 520,  586, 520,  584, 520,  586, 520,  566, 542,  590, 1616,  560, 1644,  562, 542,  562, 1650,  558, 544,  586, 520,  586, 520,  586, 522,  590, 514,  564, 542,  586, 1620,  562, 544,  562, 1648,  562, 1642,  562, 1642,  562, 1644,  588, 5262,  4488, 4398,  562, 1642,  564, 542,  564, 1646,  562, 1644,  562, 544,  584, 522,  570, 1636,  560, 546,  590, 514,  562, 1644,  562, 544,  562, 544,  562, 1644,  562, 1644,  560, 544,  562, 1650,  586, 1616,  560, 546,  560, 1650,  560, 1646,  560, 1646,  558, 1646,  560, 1646,  560, 1648,  584, 518,  560, 1650,  558, 546,  560, 548,  560, 546,  558, 548,  558, 548,  558, 550,  584, 1620,  558, 1648,  558, 548,  558, 1654,  556, 548,  558, 548,  556, 548,  558, 550,  558, 546,  558, 548,  558, 1670,  534, 550,  558, 1674,  534, 1672,  534, 1672,  532, 1676,  532};

uint16_t offSignal[199] = {4484, 4420,  562, 1670,  536, 544,  568, 1668,  534, 1670,  534, 544,  562, 544,  566, 1666,  536, 546,  590, 516,  570, 1664,  534, 544,  568, 538,  562, 1672,  534, 1670,  536, 544,  562, 1676,  562, 516,  566, 1670,  536, 1672,  536, 1670,  536, 1670,  534, 544,  584, 1652,  534, 1674,  560, 1620,  558, 544,  562, 544,  568, 538,  562, 544,  564, 1670,  534, 544,  576, 534,  590, 1640,  536, 1646,  560, 1670,  536, 544,  570, 536,  570, 536,  564, 542,  562, 546,  590, 514,  562, 544,  584, 522,  562, 1650,  556, 1648,  558, 1648,  558, 1646,  558, 1646,  588, 5268,  4490, 4396,  562, 1642,  562, 544,  564, 1646,  562, 1642,  586, 520,  564, 542,  574, 1632,  564, 546,  590, 514,  564, 1642,  562, 544,  562, 546,  562, 1644,  562, 1644,  560, 546,  560, 1650,  588, 514,  562, 1648,  560, 1646,  560, 1644,  562, 1644,  560, 546,  560, 1648,  562, 1646,  586, 1616,  560, 546,  560, 546,  560, 546,  560, 546,  560, 1644,  560, 546,  560, 548,  586, 1618,  560, 1646,  560, 1646,  558, 548,  558, 548,  558, 548,  558, 548,  560, 548,  584, 520,  558, 548,  560, 546,  558, 1648,  558, 1648,  558, 1648,  558, 1650,  556, 1652,  556};

uint8_t esp8266_mac[] = {0x08, 0xF9, 0xE0, 0x6B, 0x1C, 0xD1};
esp_now_peer_info_t peerInfo;

struct ControlRelayMessage {
  bool relayOn;
};

struct StatusMessage {
  bool relayOn;
  float currentRms;
};

DHT dht(DHTPIN, DHTTYPE);
WebServer server(80);

float temperature;
float humidity;
float power = 0;
bool relayState = false;
bool irState = false;
unsigned long lastDHTUpdate = 0;

void OnSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("MAC address: ");
  for (int i = 0; i < 6; i++) {
    if (mac_addr[i] < 0x10) {
      Serial.print("0");
    }
    Serial.print(mac_addr[i], HEX);
    if (i < 5) {
      Serial.print(":");
    }
  }
  Serial.println();
  Serial.print("Delivery Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void OnReceive(const esp_now_recv_info* recv_info, const uint8_t* incomingData, int len) {
  if (len == sizeof(StatusMessage)) {
    StatusMessage statusMessage;
    memcpy(&statusMessage, incomingData, sizeof(statusMessage));
    Serial.print("Relay State: ");
    Serial.println(statusMessage.relayOn ? "ON" : "OFF");
    Serial.print("Current RMS: ");
    Serial.println(statusMessage.currentRms, 4);
    power = statusMessage.currentRms * 220;
    relayState = statusMessage.relayOn;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(IR_SEND_PIN, OUTPUT);
  dht.begin();
  IrSender.begin(IR_SEND_PIN);

  WiFi.mode(WIFI_AP_STA);
  WiFi.setChannel(4);
  WiFi.begin(ap_ssid, ap_password);
  Serial.println("Access Point Started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Wi-Fi Channel: ");
  Serial.println(WiFi.channel());

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnSent);
  esp_now_register_recv_cb(OnReceive);

  memcpy(peerInfo.peer_addr, esp8266_mac, 6);
  peerInfo.channel = 4;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  server.on("/", HTTP_GET, handleRoot);
  server.on("/update", HTTP_GET, handleUpdate);
  server.on("/updateir", HTTP_GET, handleIR);
  server.on("/data", HTTP_GET, handleData);
  server.begin();
  Serial.println("Server started");
}

void loop() {
  server.handleClient();
  unsigned long currentMillis = millis();
  if (currentMillis - lastDHTUpdate >= 2000) {
    lastDHTUpdate = currentMillis;
    readSensorData();
  }
}

void readSensorData() {
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
}

void turnOnIR(bool on) {
  if (on) {
    IrSender.sendRaw(onSignal, sizeof(onSignal) / sizeof(onSignal[0]), 38);
    Serial.println("IR ON signal sent.");
  } else {
    IrSender.sendRaw(offSignal, sizeof(offSignal) / sizeof(offSignal[0]), 38);
    Serial.println("IR OFF signal sent.");
  }
}

void turnOnRelay(bool isOn) {
  relayState = isOn;
  ControlRelayMessage controlRelayMessage;
  controlRelayMessage.relayOn = isOn;
  esp_err_t result = esp_now_send(esp8266_mac, (uint8_t *)&controlRelayMessage, sizeof(controlRelayMessage));
  Serial.printf("Relay %s message sent\n", isOn ? "ON" : "OFF");
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  } else {
    Serial.println("Error sending the data");
  }
}

void handleRoot() {
  String html = "<html><head><title>ESP32 Web Server</title></head><body>";
  html += "<h1>ESP32 Home Controller</h1>";
  html += "<p>Temperature: <span id='temp'></span> &#8451;</p>";
  html += "<p>Humidity: <span id='hum'></span> %</p>";
  html += "<p>Power: <span id='power'></span> W</p>";

  html += "<p>Relay control</p>";
  html += "<button onclick=\"toggleRelay('on')\">Turn On Relay</button>";
  html += "<button onclick=\"toggleRelay('off')\">Turn Off Relay</button>";
  html += "<p>IR remote</p>";
  html += "<button onclick=\"toggleIR('on')\">Turn On IR</button>";
  html += "<button onclick=\"toggleIR('off')\">Turn Off IR</button>";

  html += "<script>";
  html += "function updateData() {";
  html += "  fetch('/data').then(response => response.json()).then(data => {";
  html += "    document.getElementById('temp').innerText = data.temperature;";
  html += "    document.getElementById('hum').innerText = data.humidity;";
  html += "    document.getElementById('power').innerText = data.power;";
  html += "  });";
  html += "}";
  html += "setInterval(updateData, 1000);";  // Cập nhật mỗi giây
  html += "function toggleRelay(state) {";
  html += "  fetch('/update?relay=' + state);";
  html += "}";
  html += "function toggleIR(state) {";
  html += "  fetch('/updateir?ir=' + state);";
  html += "}";
  html += "</script>";
  
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

void handleUpdate() {
  if (server.hasArg("relay")) {
    String relayStateArg = server.arg("relay");
    if (relayStateArg == "on") {
      turnOnRelay(true);
    } else if (relayStateArg == "off") {
      turnOnRelay(false);
    }
  }
  server.send(200, "text/plain", "Relay state updated");
}

void handleIR() {
  if (server.hasArg("ir")) {
    String irStateArg = server.arg("ir");
    if (irStateArg == "on") {
      turnOnIR(true);
    } else if (irStateArg == "off") {
      turnOnIR(false);
    }
  }
  server.send(200, "text/plain", "Relay state updated");
}

void handleData() {
  String json = "{\"temperature\":" + String(temperature, 1) + 
                ", \"humidity\":" + String(humidity, 1) + 
                ", \"power\":" + String(power) + "}";
  server.send(200, "application/json", json);
}
