#include <WiFi.h>
#include <esp_now.h>

uint8_t esp8266_mac[] = {0x08, 0xF9, 0xE0, 0x6B, 0x1C, 0xD1};  // Địa chỉ MAC của ESP8266

struct ControlRelayMessage {
  bool relayOn;
};

struct StatusMessage {
  bool relayOn;
  float currentRms;
};

esp_now_peer_info_t peerInfo;

unsigned long lastSendTime = 0;
const unsigned long SEND_INTERVAL = 2000; // 2 giây gửi 1 lần

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success"
                                                : "Delivery Fail");
}

// Chỉnh sửa hàm onDataReceive để phù hợp với định nghĩa mới
void onDataReceive(const esp_now_recv_info* recv_info, const uint8_t* incomingData, int len) {
  if (len == sizeof(StatusMessage)) {
    StatusMessage statusMessage;
    memcpy(&statusMessage, incomingData, sizeof(statusMessage));
    Serial.print("Relay State: ");
    Serial.println(statusMessage.relayOn ? "ON" : "OFF");
    Serial.print("Current RMS: ");
    Serial.println(statusMessage.currentRms, 4);
  }
}

void setup() {
  // Init Serial Monitor
  Serial.begin(115200);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register callback functions
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(onDataReceive);

  // Register peer
  memcpy(peerInfo.peer_addr, esp8266_mac, 6);
  peerInfo.channel = 1;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
}

void loop() {
  unsigned long currentMillis = millis();

  // Gửi trạng thái relay mỗi 2 giây
  if (currentMillis - lastSendTime >= SEND_INTERVAL) {
    lastSendTime = currentMillis;

    ControlRelayMessage controlRelayMessage;
    controlRelayMessage.relayOn = true;  // Bật relay

    // Gửi thông điệp điều khiển relay
    esp_err_t result = esp_now_send(esp8266_mac, (uint8_t *)&controlRelayMessage, sizeof(controlRelayMessage));

    if (result == ESP_OK) {
      Serial.println("Sent with success");
    } else {
      Serial.println("Error sending the data");
    }

    // Gửi thông điệp điều khiển relay để tắt relay sau 2 giây
    delay(2000);

    controlRelayMessage.relayOn = false;

    result = esp_now_send(esp8266_mac, (uint8_t *)&controlRelayMessage, sizeof(controlRelayMessage));

    if (result == ESP_OK) {
      Serial.println("Sent with success");
    } else {
      Serial.println("Error sending the data");
    }
  }
}
