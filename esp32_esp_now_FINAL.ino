#define BLYNK_TEMPLATE_ID "TMPL60TfWkyOC"
#define BLYNK_TEMPLATE_NAME "TEST ESP32 DHT11"
#define BLYNK_AUTH_TOKEN "5TNG-NgCd6G9kDI2Q2wrzIIVxukfJkUV"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <esp_now.h>

#define DHTPIN 4           // GPIO4 connected to DHT11
#define DHTTYPE DHT11
#define VPIN_RELAY V2      // Virtual pin for relay control
#define VPIN_TEMP V0       // Virtual pin for temperature
#define VPIN_HUMIDITY V1   // Virtual pin for humidity

char ssid[] = "Shodydosh";
char pass[] = "25022003";
char auth[] = BLYNK_AUTH_TOKEN;

DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

// ESP8266 MAC address (update with your ESP8266 MAC address)
uint8_t esp8266Address[] = {0x08, 0xF9, 0xE0, 0x6B, 0x1C, 0xD1};

typedef struct struct_message {
    bool relayState;
} struct_message;

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