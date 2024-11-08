#define BLYNK_TEMPLATE_ID "TMPL60TfWkyOC"
#define BLYNK_TEMPLATE_NAME "TEST ESP32 DHT11"
#define BLYNK_AUTH_TOKEN "5TNG-NgCd6G9kDI2Q2wrzIIVxukfJkUV"

#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

#define DHTPIN 4                          // Pin connected to the DHT11 sensor
#define DHTTYPE DHT11                     // DHT11 sensor type
#define WIFI_SSID "Shodydosh"              // Replace with your Wi-Fi SSID
#define WIFI_PASS "25022003"          // Replace with your Wi-Fi password

DHT dht(DHTPIN, DHTTYPE);

void setup() {
    Serial.begin(115200);
    dht.begin();
    
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.print("Connecting to Wi-Fi...");
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nConnected to Wi-Fi.");
}

void sendToBlynk(float temperature, float humidity) {
    if (WiFi.status() == WL_CONNECTED) {  // Check if Wi-Fi is connected
        HTTPClient http;

        // Send Temperature
        String urlTemp = String("https://sgp1.blynk.cloud/external/api/update?token=") + BLYNK_AUTH_TOKEN + "&TEMP1=" + temperature;
        http.begin(urlTemp);
        int httpResponseCodeTemp = http.GET();
        if (httpResponseCodeTemp > 0) {
            Serial.println("Temperature sent successfully.");
        } else {
            Serial.println("Error sending temperature.");
        }
        http.end();

        // Send Humidity
        String urlHum = String("https://sgp1.blynk.cloud/external/api/update?token=") + BLYNK_AUTH_TOKEN + "&HUMID1=" + humidity;
        http.begin(urlHum);
        int httpResponseCodeHum = http.GET();
        if (httpResponseCodeHum > 0) {
            Serial.println("Humidity sent successfully.");
        } else {
            Serial.println("Error sending humidity.");
        }
        http.end();
    } else {
        Serial.println("Not connected to Wi-Fi.");
    }
}

void loop() {
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

    if (isnan(humidity) || isnan(temperature)) {
        Serial.println("Failed to read from DHT sensor!");
    } else {
        Serial.print("Temperature: ");
        Serial.print(temperature);
        Serial.print(" °C, Humidity: ");
        Serial.print(humidity);
        Serial.println(" %");

        // Send data to Blynk
        sendToBlynk(temperature, humidity);
    }

    delay(10000);  // Delay for 10 seconds before sending data again
}

