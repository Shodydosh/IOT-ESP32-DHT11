#include <ESP8266WiFi.h>
#include <espnow.h>

const int relayPin = 4;

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
  bool relayOn;
} struct_message;

// Create a struct_message called myData
struct_message myData;

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.println(myData.relayOn);
  if (myData.relayOn == 1) {
    digitalWrite(relayPin, HIGH);
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("On");
  } else {
    digitalWrite(relayPin, LOW);
    digitalWrite(LED_BUILTIN, LOW);
    Serial.println("Off");
  }
  Serial.println();
}

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(relayPin, OUTPUT);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
}

void loop() {}