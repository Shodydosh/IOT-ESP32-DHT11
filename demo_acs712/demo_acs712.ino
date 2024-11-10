const int analogPin = A0;
const float VREF = 5;                // Điện áp tham chiếu (V)
const int ADC_RESOLUTION = 1024;     // Độ phân giải ADC của ESP8266 (10-bit)
const float SENSOR_SENSITIVITY = 0.1; // Độ nhạy của ACS712 20A(0.1V/A) 5A(0.185V/A) 30A(0.066V/A)

const int sampleCount = 1000;        // Số lần lấy mẫu để tính RMS
float voltageOffset = VREF / 2;      // Điện áp trung bình (offset khi tải đầu vào không có điện), ban đầu giả định là VREF / 2

void setup() {
  Serial.begin(9600);
  while (!Serial);                  //Ngắt thiết bị đo/tải để cân bằng lại cảm biến
  calibrateOffset();                //Cân bằng lại điện áp trung bình mà chân A0 thu được từ cảm biến
  Serial.println("Calibrated voltage offset: " + String(voltageOffset, 3) + " V");
}

void loop() {
  float currentRMS = measureCurrentRMS();
  Serial.print("Dòng điện AC RMS: ");
  Serial.print(currentRMS, 4);
  Serial.println(" A");

  delay(1000);  // Đọc lại mỗi giây
}

// Hàm lấy mẫu điện áp trung bình để xác định offset
void calibrateOffset() {
  long sum = 0;
  for (int i = 0; i < sampleCount; i++) {
    sum += analogRead(analogPin);
    delay(1);
  }
  float avgADC = sum / (float)sampleCount;
  voltageOffset = (avgADC / ADC_RESOLUTION) * VREF;
}

// Hàm đo dòng điện AC RMS
float measureCurrentRMS() {
  float sum = 0.0;
  
  for (int i = 0; i < sampleCount; i++) {
    int analogValue = analogRead(analogPin);
    float voltage = (analogValue / (float)ADC_RESOLUTION) * VREF;
    float voltageDiff = voltage - voltageOffset;
    sum += voltageDiff * voltageDiff;
    delay(1);
  }

  float voltageRMS = sqrt(sum / sampleCount);
  float currentRMS = voltageRMS / SENSOR_SENSITIVITY;

  return currentRMS;
}
