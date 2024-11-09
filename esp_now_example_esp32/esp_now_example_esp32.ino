#include <WiFi.h>
#include <esp_now.h>

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t receiver8266Address[] = {0x08, 0xF9, 0xE0, 0x6B, 0x1C, 0xD1};

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  bool relayOn;
} struct_message;

// Create a struct_message called myData
struct_message myData;

esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success"
                                                : "Delivery Fail");
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

  // Once ESPNow is successfully Init, we will register for Send Callback func to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  memcpy(peerInfo.peer_addr, receiver8266Address, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
}

void loop() {
  delay(2000);
  // Set values to send
  myData.relayOn = true;

  // Send message via ESP-NOW
  esp_err_t result =
      esp_now_send(receiver8266Address, (uint8_t *)&myData, sizeof(myData));

  if (result == ESP_OK) {
    Serial.println("Sent with success");
  } else {
    Serial.println("Error sending the data: ", result);
  }

  delay(2000);

  myData.relayOn = false;

  result = esp_now_send(receiver8266Address, (uint8_t *)&myData, sizeof(myData));

  if (result == ESP_OK) {
    Serial.println("Sent with success");
  } else {
    Serial.println("Error sending the data: ", result);
  }
}