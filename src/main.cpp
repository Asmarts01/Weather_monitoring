#include <ESP8266WiFi.h>
#include <Adafruit_BMP280.h>

// WiFi credentials
const char* ssid = "Redmi 13C";
const char* password = "what a hassle";

// Create BMP280 object
Adafruit_BMP280 bmp;

// Rain sensor pins
#define RAIN_ANALOG A0
#define RAIN_DIGITAL D5

WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  delay(10);

  // Connect WiFi
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Start server
  server.begin();

  // Initialize BMP280 with dual address check
  if (!bmp.begin(0x76)) {
    if (!bmp.begin(0x77)) {
      Serial.println("Could not find BMP280 sensor at 0x76 or 0x77!");
      while (1);
    }
  }

  pinMode(RAIN_DIGITAL, INPUT);
}

void loop() {
  WiFiClient client = server.accept();
  if (!client) return;

  while (!client.available()) delay(1);
  String request = client.readStringUntil('\r');
  client.flush();

  // Serve JSON sensor data
  if (request.indexOf("/data") != -1) {
    float temperature = bmp.readTemperature();
    float pressure = bmp.readPressure() / 100.0F; // hPa
    float altitude = bmp.readAltitude(1013.25);   // meters
    int rainDigital = digitalRead(RAIN_DIGITAL);

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.println();
    client.print("{\"temperature\":");
    client.print(temperature);
    client.print(",\"pressure\":");
    client.print(pressure);
    client.print(",\"altitude\":");
    client.print(altitude);
    client.print(",\"rain\":");
    client.print(rainDigital);
    client.print("}");
    return;
  }

    // Serve main HTML page
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html; charset=utf-8");
  client.println("Connection: close");
  client.println();
  client.println("<!DOCTYPE HTML><html><head>");
  client.println("<title>ESP8266 Weather Monitoring System</title>");
  client.println("<meta name='viewport' content='width=device-width, initial-scale=1'>");
  client.println("<style>");
  client.println("body { font-family: Arial; background:#eef2f3; text-align:center; margin:0; }");
  client.println(".card { background:#fff; border-radius:15px; padding:20px; margin:15px; box-shadow:0 4px 8px rgba(0,0,0,0.2); }");
  client.println("h2 { color:#333; margin-top:20px; }");
  client.println("p { font-size:22px; margin:8px; font-weight:bold; }");
  client.println("</style>");
  client.println("<script>");
  client.println("function updateData(){");
  client.println("fetch('/data').then(r=>r.json()).then(d=>{");
  client.println("document.getElementById('temp').innerText = d.temperature.toFixed(2)+' °C';");
  client.println("document.getElementById('press').innerText = d.pressure.toFixed(2)+' hPa';");
  client.println("document.getElementById('alt').innerText = d.altitude.toFixed(2)+' m';");
  client.println("document.getElementById('rain').innerText = (d.rain==0 ? 'RAIN detected' : 'Dry');");
  client.println("document.getElementById('rain').style.color = (d.rain==0 ? 'red' : 'blue');");
  client.println("});}");
  client.println("window.onload = ()=>{ updateData(); setInterval(updateData, 1000); };");
  client.println("</script></head><body>");
  client.println("<h2>HARVARDE COLLEGE<br>ESP8266 Weather Monitoring System</h2>");
  client.println("<div class='card'><p>Temperature: <span id='temp'>-- °C</span></p></div>");
  client.println("<div class='card'><p>Pressure: <span id='press'>-- hPa</span></p></div>");
  client.println("<div class='card'><p>Altitude: <span id='alt'>-- m</span></p></div>");
  client.println("<div class='card'><p>Rain Status: <span id='rain'>--</span></p></div>");
  client.println("</body></html>");
}