// Blynk Tung
// #define BLYNK_TEMPLATE_ID "TMPL60TfWkyOC"
// #define BLYNK_TEMPLATE_NAME "TEST ESP32 DHT11"
// #define BLYNK_AUTH_TOKEN "5TNG-NgCd6G9kDI2Q2wrzIIVxukfJkUV"

// Blynk Quan
#define BLYNK_TEMPLATE_ID "TMPL6xU41P2DF"
#define BLYNK_TEMPLATE_NAME "IOT BTL"
#define BLYNK_AUTH_TOKEN "uIbs7qaLUo70LAtCcByyqSGzZxglsMC5"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <esp_now.h>

#define DHTPIN 4           // GPIO4 connected to DHT11
#define DHTTYPE DHT11
#define VPIN_RELAY V2      // Virtual pin for relay control
#define VPIN_TEMP V0       // Virtual pin for temperature
#define VPIN_HUMIDITY V1   // Virtual pin for humidity

uint16_t onSignal[199] = {4488, 4396,  562, 1670,  534, 544,  586, 1650,  536, 1670,  536, 544,  566, 540,  568, 1642,  556, 546,  590, 514,  586, 1646,  536, 544,  574, 532,  562, 1670,  536, 1670,  534, 544,  562, 1654,  584, 1618,  558, 544,  586, 1652,  534, 1670,  536, 1646,  558, 1670,  536, 1670,  536, 1672,  562, 516,  582, 1630,  558, 544,  586, 520,  586, 520,  584, 520,  586, 520,  566, 542,  590, 1616,  560, 1644,  562, 542,  562, 1650,  558, 544,  586, 520,  586, 520,  586, 522,  590, 514,  564, 542,  586, 1620,  562, 544,  562, 1648,  562, 1642,  562, 1642,  562, 1644,  588, 5262,  4488, 4398,  562, 1642,  564, 542,  564, 1646,  562, 1644,  562, 544,  584, 522,  570, 1636,  560, 546,  590, 514,  562, 1644,  562, 544,  562, 544,  562, 1644,  562, 1644,  560, 544,  562, 1650,  586, 1616,  560, 546,  560, 1650,  560, 1646,  560, 1646,  558, 1646,  560, 1646,  560, 1648,  584, 518,  560, 1650,  558, 546,  560, 548,  560, 546,  558, 548,  558, 548,  558, 550,  584, 1620,  558, 1648,  558, 548,  558, 1654,  556, 548,  558, 548,  556, 548,  558, 550,  558, 546,  558, 548,  558, 1670,  534, 550,  558, 1674,  534, 1672,  534, 1672,  532, 1676,  532};
uint16_t offSignal[199] = {4484, 4420,  562, 1670,  536, 544,  568, 1668,  534, 1670,  534, 544,  562, 544,  566, 1666,  536, 546,  590, 516,  570, 1664,  534, 544,  568, 538,  562, 1672,  534, 1670,  536, 544,  562, 1676,  562, 516,  566, 1670,  536, 1672,  536, 1670,  536, 1670,  534, 544,  584, 1652,  534, 1674,  560, 1620,  558, 544,  562, 544,  568, 538,  562, 544,  564, 1670,  534, 544,  576, 534,  590, 1640,  536, 1646,  560, 1670,  536, 544,  570, 536,  570, 536,  564, 542,  562, 546,  590, 514,  562, 544,  584, 522,  562, 1650,  556, 1648,  558, 1648,  558, 1646,  558, 1646,  588, 5268,  4490, 4396,  562, 1642,  562, 544,  564, 1646,  562, 1642,  586, 520,  564, 542,  574, 1632,  564, 546,  590, 514,  564, 1642,  562, 544,  562, 546,  562, 1644,  562, 1644,  560, 546,  560, 1650,  588, 514,  562, 1648,  560, 1646,  560, 1644,  562, 1644,  560, 546,  560, 1648,  562, 1646,  586, 1616,  560, 546,  560, 546,  560, 546,  560, 546,  560, 1644,  560, 546,  560, 548,  586, 1618,  560, 1646,  560, 1646,  558, 548,  558, 548,  558, 548,  558, 548,  560, 548,  584, 520,  558, 548,  560, 546,  558, 1648,  558, 1648,  558, 1648,  558, 1650,  556, 1652,  556};

char ssid[] = "OPPO A15s";
char pass[] = "67891234";
char auth[] = BLYNK_AUTH_TOKEN;

DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

// ESP8266 MAC address (update with your ESP8266 MAC address)
uint8_t esp8266Address[] = {0x08, 0xF9, 0xE0, 0x6B, 0x1C, 0xD1};

struct ControlRelayMessage {
  bool relayOn;
};

struct StatusMessage {
  bool relayOn;
  float currentRms;
};

struct_message relayMessage;

void onSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// Send DHT11 data to Blynk
void sendSensorData() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Blynk.virtualWrite(VPIN_TEMP, t);       // Send temperature to Blynk
  Blynk.virtualWrite(VPIN_HUMIDITY, h);   // Send humidity to Blynk

  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" °C    Humidity: ");
  Serial.print(h);
  Serial.println(" %");
}

// Relay control function triggered by Blynk
BLYNK_WRITE(VPIN_RELAY) {
  relayMessage.relayState = param.asInt() != 0;
  esp_err_t result = esp_now_send(esp8266Address, (uint8_t *) &relayMessage, sizeof(relayMessage));

  if (result == ESP_OK) {
    Serial.println("Relay command sent successfully");
  } else {
    Serial.println("Error sending relay command");
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();

  WiFi.begin(ssid, pass);
  int wifiAttempts = 0;
  while (WiFi.status() != WL_CONNECTED && wifiAttempts < 20) {
    delay(1000);
    Serial.print(".");
    wifiAttempts++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected");
  } else {
    Serial.println("Failed to connect to WiFi");
    return;
  }

  Blynk.begin(auth, ssid, pass);
  timer.setInterval(2000L, sendSensorData); // Send DHT data every 2 seconds

  // ESP-NOW setup
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  esp_now_register_send_cb(onSent);
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, esp8266Address, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

}

void loop() {
  Blynk.run();
  timer.run();
}