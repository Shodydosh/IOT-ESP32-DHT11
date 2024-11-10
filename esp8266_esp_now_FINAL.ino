#include <ESP8266WiFi.h>
#include <espnow.h>

const int ANALOG_PIN = A0;
const int RELAY_PIN = 4;
const float VREF = 5.0;
const int ADC_RESOLUTION = 1024;
const float SENSOR_SENSITIVITY = 0.1;
const int SAMPLE_COUNT = 1000;
const int SAMPLE_INTERVAL = 1;
const float CURRENT_THRESHOLD = 0.02;  // Ngưỡng dòng điện đo được để xác định trạng thái bật/tắt
const unsigned long SEND_INTERVAL = 30000;

float voltageOffset = VREF / 2;
unsigned long lastSampleTime = 0;
int sampleCounter = 0;
float sum = 0.0;
float currentRMS = 0.0;

unsigned long lastSendTime = 0;
bool currentRelayState = false;
bool lastRelayState = false;

uint8_t esp32_mac[] = {0x10, 0x06, 0x1C, 0x86, 0xA7, 0x08};  // Địa chỉ MAC của ESP32

struct ControlRelayMessage {
  bool relayOn;
};

struct StatusMessage {
  bool relayOn;
  float currentRms;
};

void setup() {
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(LED_BUILTIN, LOW);


  calibrateOffset();
  Serial.println("Calibrated voltage offset: " + String(voltageOffset, 3) + " V");

  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);

  esp_now_register_send_cb(OnDataSent);

  esp_now_register_recv_cb(onDataReceive);

  // Đăng ký MAC address của ESP32 để gửi dữ liệu tới ESP32
  esp_now_add_peer(esp32_mac, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
}

void loop() {
  measureCurrentRMS();
  if (sampleCounter >= SAMPLE_COUNT) {
    currentRMS = calculateCurrentRMS();
  }

  // Gửi giá trị RMS mỗi 30 giây
  if (millis() - lastSendTime >= SEND_INTERVAL) {
    lastSendTime = millis();
    sendStatusMessage(currentRelayState, currentRMS);
  }

  // Kiểm tra trạng thái thiết bị dựa trên ngưỡng dòng điện
  bool currentRelayState = (currentRMS >= CURRENT_THRESHOLD);
  if (currentRelayState != lastRelayState) {
    lastRelayState = currentRelayState;
    sendStatusMessage(currentRelayState, currentRMS);
  }
}

// Hàm lấy mẫu điện áp trung bình để xác định offset
void calibrateOffset() {
  long sum = 0;
  for (int i = 0; i < SAMPLE_COUNT; i++) {
    sum += analogRead(ANALOG_PIN);
    delay(1);
  }
  float avgADC = sum / (float)SAMPLE_COUNT;
  voltageOffset = (avgADC / ADC_RESOLUTION) * VREF;
}

// Hàm lấy mẫu RMS
void measureCurrentRMS() {
  if (millis() - lastSampleTime >= SAMPLE_INTERVAL) {
    lastSampleTime = millis();
    
    int analogValue = analogRead(ANALOG_PIN);
    float voltage = (analogValue / (float)ADC_RESOLUTION) * VREF;
    float voltageDiff = voltage - voltageOffset;
    sum += voltageDiff * voltageDiff;
    sampleCounter++;
  }
}

// Hàm tính RMS từ các mẫu
float calculateCurrentRMS() {
  float voltageRMS = sqrt(sum / sampleCounter);
  float currentRMS = voltageRMS / SENSOR_SENSITIVITY;
  
  sum = 0.0;
  sampleCounter = 0;
  Serial.print("Current RMS: ");
  Serial.println(currentRMS, 4);
  
  return currentRMS;
}

// Gửi trạng thái thiết bị qua ESP-NOW
void sendStatusMessage(bool relayState, float currentRMS) {
  StatusMessage statusMessage;
  statusMessage.relayOn = relayState;
  statusMessage.currentRms = currentRMS;
  
  uint8_t message[sizeof(StatusMessage)];
  memcpy(message, &statusMessage, sizeof(statusMessage));
  
  esp_now_send(esp32_mac, message, sizeof(message));  // Gửi đến ESP32
  
  Serial.print("Relay State: ");
  Serial.println(relayState ? "ON" : "OFF");
  Serial.print("Current RMS: ");
  Serial.println(currentRMS, 4);
}

// Callback ESP-NOW để nhận dữ liệu điều khiển
void onDataReceive(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  if (len == sizeof(ControlRelayMessage)) {
    ControlRelayMessage controlMessage;
    memcpy(&controlMessage, incomingData, sizeof(controlMessage));

    if (controlMessage.relayOn) {
      digitalWrite(LED_BUILTIN, HIGH);
      digitalWrite(RELAY_PIN, HIGH);
      Serial.println("Relay State: ON");
    } else {
      digitalWrite(LED_BUILTIN, LOW);
      digitalWrite(RELAY_PIN, LOW);
      Serial.println("Relay State: OFF");
    }
  }
}

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    Serial.println("Delivery success");
  } else {
    Serial.println("Delivery fail");
  }
}
