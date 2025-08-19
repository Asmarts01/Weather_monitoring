#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_Sensor.h>
#include <ESP8266WiFi.h>

// ================================
// Pin Definitions
// ================================
#define SDA_PIN 4      // GPIO4 (D2)
#define SCL_PIN 5      // GPIO5 (D1)
#define RAIN_DIGITAL 14  // GPIO14 (D5)
#define RAIN_ANALOG A0   // AO -> A0 (through divider)

// Voltage Divider
const float Rt = 30000.0;
const float Rb = 10000.0;

// ================================
// Wi-Fi Config
// ================================
const char* ssid = "Redmi 13C";       // <-- change this
const char* password = "what a hassle";   // <-- change this

WiFiServer server(80);

// ================================
// Sensors
// ================================
Adafruit_BMP280 bmp;

float adcToSensorVoltage(int adc) {
  float v_adc = (adc / 1023.0) * 1.0;
  return v_adc * (Rt + Rb) / Rb;
}

void setup() {
  Serial.begin(115200);
  delay(100);

  Wire.begin(SDA_PIN, SCL_PIN);

  // BMP280 init
  if (!bmp.begin(0x76)) {
    if (!bmp.begin(0x77)) {
      Serial.println("⚠️ Could not find BMP280 sensor!");
      while (1);
    }
  }

  pinMode(RAIN_DIGITAL, INPUT);

  // Wi-Fi connect
  Serial.print("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n Wi-Fi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (!client) return;

  while (!client.available()) delay(1);

  String request = client.readStringUntil('\r');
  client.flush();

  // Read sensors
  int rainAnalog = analogRead(RAIN_ANALOG);
  int rainDigital = digitalRead(RAIN_DIGITAL);
  float sensorV = adcToSensorVoltage(rainAnalog);

  float temperature = bmp.readTemperature();
  float pressure = bmp.readPressure() / 100.0F;
  float altitude = bmp.readAltitude(1013.25);

  // Send HTML page
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<h2>🌦 ESP8266 Weather Station</h2>");
  client.println("<p>Temperature: " + String(temperature) + " °C</p>");
  client.println("<p>Pressure: " + String(pressure) + " hPa</p>");
  client.println("<p>Altitude: " + String(altitude) + " m</p>");
  client.println("<p>Rain Analog: " + String(rainAnalog) + " (" + String(sensorV, 3) + " V)</p>");
  client.print("<p>Rain Status: ");
  client.print((rainDigital == 0) ? "🌧 RAIN detected" : "☀️ Dry");
  client.println("</p>");
  client.println("</html>");
}
