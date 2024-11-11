// Blynk Quan
#define BLYNK_TEMPLATE_ID "TMPL6xU41P2DF"
#define BLYNK_TEMPLATE_NAME "IOT BTL"
#define BLYNK_AUTH_TOKEN "uIbs7qaLUo70LAtCcByyqSGzZxglsMC5"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <esp_now.h>
#include <IRremote.hpp>

#define DHTPIN 14           // GPIO4 connected to DHT11
#define IR_SEND_PIN 18     // GPIO18 connected to IR LED
#define DHTTYPE DHT11
#define VPIN_RELAY V2      // Virtual pin for relay control
#define VPIN_TEMP V0       // Virtual pin for temperature
#define VPIN_HUMIDITY V1   // Virtual pin for humidity
#define VPIN_POWER V3      // Virtual pin for power consumption
#define VPIN_IR_CONTROL V4 // Virtual pin for IR remote control

uint16_t onSignal[199] = {4488, 4396,  562, 1670,  534, 544,  586, 1650,  536, 1670,  536, 544,  566, 540,  568, 1642,  556, 546,  590, 514,  586, 1646,  536, 544,  574, 532,  562, 1670,  536, 1670,  534, 544,  562, 1654,  584, 1618,  558, 544,  586, 1652,  534, 1670,  536, 1646,  558, 1670,  536, 1670,  536, 1672,  562, 516,  582, 1630,  558, 544,  586, 520,  586, 520,  584, 520,  586, 520,  566, 542,  590, 1616,  560, 1644,  562, 542,  562, 1650,  558, 544,  586, 520,  586, 520,  586, 522,  590, 514,  564, 542,  586, 1620,  562, 544,  562, 1648,  562, 1642,  562, 1642,  562, 1644,  588, 5262,  4488, 4398,  562, 1642,  564, 542,  564, 1646,  562, 1644,  562, 544,  584, 522,  570, 1636,  560, 546,  590, 514,  562, 1644,  562, 544,  562, 544,  562, 1644,  562, 1644,  560, 544,  562, 1650,  586, 1616,  560, 546,  560, 1650,  560, 1646,  560, 1646,  558, 1646,  560, 1646,  560, 1648,  584, 518,  560, 1650,  558, 546,  560, 548,  560, 546,  558, 548,  558, 548,  558, 550,  584, 1620,  558, 1648,  558, 548,  558, 1654,  556, 548,  558, 548,  556, 548,  558, 550,  558, 546,  558, 548,  558, 1670,  534, 550,  558, 1674,  534, 1672,  534, 1672,  532, 1676,  532};

uint16_t offSignal[199] = {4484, 4420,  562, 1670,  536, 544,  568, 1668,  534, 1670,  534, 544,  562, 544,  566, 1666,  536, 546,  590, 516,  570, 1664,  534, 544,  568, 538,  562, 1672,  534, 1670,  536, 544,  562, 1676,  562, 516,  566, 1670,  536, 1672,  536, 1670,  536, 1670,  534, 544,  584, 1652,  534, 1674,  560, 1620,  558, 544,  562, 544,  568, 538,  562, 544,  564, 1670,  534, 544,  576, 534,  590, 1640,  536, 1646,  560, 1670,  536, 544,  570, 536,  570, 536,  564, 542,  562, 546,  590, 514,  562, 544,  584, 522,  562, 1650,  556, 1648,  558, 1648,  558, 1646,  558, 1646,  588, 5268,  4490, 4396,  562, 1642,  562, 544,  564, 1646,  562, 1642,  586, 520,  564, 542,  574, 1632,  564, 546,  590, 514,  564, 1642,  562, 544,  562, 546,  562, 1644,  562, 1644,  560, 546,  560, 1650,  588, 514,  562, 1648,  560, 1646,  560, 1644,  562, 1644,  560, 546,  560, 1648,  562, 1646,  586, 1616,  560, 546,  560, 546,  560, 546,  560, 546,  560, 1644,  560, 546,  560, 548,  586, 1618,  560, 1646,  560, 1646,  558, 548,  558, 548,  558, 548,  558, 548,  560, 548,  584, 520,  558, 548,  560, 546,  558, 1648,  558, 1648,  558, 1648,  558, 1650,  556, 1652,  556};

char ssid[] = "OPPO A15s";
char pass[] = "67891234";
char auth[] = BLYNK_AUTH_TOKEN;

DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

// ESP8266 MAC address
uint8_t esp8266Address[] = {0x08, 0xF9, 0xE0, 0x6B, 0x1C, 0xD1};
esp_now_peer_info_t peerInfo;

struct ControlRelayMessage {
  bool relayOn;
};

struct StatusMessage {
  bool relayOn;
  float currentRms;
};

void onSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
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
  Serial.print("Delivery Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void onReceive(const esp_now_recv_info* recv_info, const uint8_t* incomingData, int len) {
  if (len == sizeof(StatusMessage)) {
    StatusMessage statusMessage;
    memcpy(&statusMessage, incomingData, sizeof(statusMessage));
    Serial.print("Relay State: ");
    Serial.println(statusMessage.relayOn ? "ON" : "OFF");
    Serial.print("Current RMS: ");
    Serial.println(statusMessage.currentRms, 4);
    Blynk.virtualWrite(VPIN_POWER, statusMessage.currentRms * 220);
  }
}

void sendRelayControl(bool on) {
  ControlRelayMessage control;
  control.relayOn = on;
  esp_err_t result = esp_now_send(esp8266Address, (uint8_t *)&control, sizeof(control));
  Serial.printf("Relay %s message sent\n", on ? "ON" : "OFF");
  if (result == ESP_OK) {
      Serial.println("Sent with success");
    } else {
      Serial.println("Error sending the data");
  }
}

void sendSensorData() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (!isnan(temperature) && !isnan(humidity)) {
    Blynk.virtualWrite(VPIN_TEMP, temperature-1);
    Blynk.virtualWrite(VPIN_HUMIDITY, humidity);
  } else {
    Serial.println("Failed to read from DHT sensor!");
  }
}

void controlIR(bool on) {
  if (on) {
    IrSender.sendRaw(onSignal, sizeof(onSignal) / sizeof(onSignal[0]), 38);
    Serial.println("IR ON signal sent.");
  } else {
    IrSender.sendRaw(offSignal, sizeof(offSignal) / sizeof(offSignal[0]), 38);
    Serial.println("IR OFF signal sent.");
  }
}

BLYNK_WRITE(VPIN_RELAY) {
  bool relayOn = param.asInt();
  sendRelayControl(relayOn);
}

BLYNK_WRITE(VPIN_IR_CONTROL) {
  bool irOn = param.asInt();
  controlIR(irOn);
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  IrSender.begin(IR_SEND_PIN);

  Blynk.begin(auth, ssid, pass);
  timer.setInterval(2000L, sendSensorData);

  WiFi.mode(WIFI_STA);
  Serial.print("Wifi channel: ");
  Serial.println(WiFi.channel());

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(onSent);
  esp_now_register_recv_cb(onReceive);

  memcpy(peerInfo.peer_addr, esp8266Address, 6);
  peerInfo.channel = 11;
  peerInfo.encrypt = false;
  Serial.print("Add peer status: ");
  Serial.println(esp_now_add_peer(&peerInfo));
}

void loop() {
  Blynk.run();
  timer.run();
}
