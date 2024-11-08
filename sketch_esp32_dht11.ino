#define BLYNK_TEMPLATE_ID "TMPL60TfWkyOC"
#define BLYNK_TEMPLATE_NAME "TEST ESP32 DHT11"
#define BLYNK_AUTH_TOKEN "5TNG-NgCd6G9kDI2Q2wrzIIVxukfJkUV"

// QUAN
// #define BLYNK_TEMPLATE_ID "TMPL6xU41P2DF"
// #define BLYNK_TEMPLATE_NAME "IOT BTL"
// #define BLYNK_AUTH_TOKEN "uIbs7qaLUo70LAtCcByyqSGzZxglsMC5"

#include <WiFi.h>  // Thư viện WiFi cho ESP32
#include <BlynkSimpleEsp32.h>  // Thư viện Blynk cho ESP32
#include <DHT.h>

#define DHTPIN 4  // GPIO4 tương ứng với D04 trên ESP32
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

char ssid[] = "Shodydosh";
char pass[] = "25022003";
char auth[] = BLYNK_AUTH_TOKEN;

BlynkTimer timer;

void sendSensorData() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Blynk.virtualWrite(V0, t); // V0 là chân ảo cho nhiệt độ
  Blynk.virtualWrite(V1, h); // V1 là chân ảo cho độ ẩm

  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" °C    Humidity: ");
  Serial.print(h);
  Serial.println(" %");
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
  timer.setInterval(2000L, sendSensorData);
}

void loop() {
  Blynk.run();
  timer.run();
}

