#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "khaja-esp";
const char* password = "12345678";

IPAddress local_ip(192, 168, 10, 1);
IPAddress gateway(192, 168, 10, 1);
IPAddress subnet(255, 255, 255, 0);

WebServer server(80);

int in1 = 13;
int in2 = 12;

// Webpage HTML
const char MAIN_page[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 LED Control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial; text-align: center; margin-top: 50px; background: #111; color: white; }
    button { padding: 20px 40px; font-size: 20px; margin: 20px; border: none; border-radius: 10px; }
    .up { background-color: black; color: white; }
    .down { background-color: black; color: white; }
    .calibrate { background-color: red; color: white }
  </style>
</head>
<body>
  <h1>Actuator control</h1>
  <button class="up" onclick="fetch('/actuator/up')">Up</button>
  <button class="down" onclick="fetch('/actuator/down')">Down</button>
  <button class="calibrate" onclick="fetch('/actuator/calibrate')">Calibrate</button>
</body>
</html>
)rawliteral";

// Route handlers
void handleRoot() {
  server.send(200, "text/html", MAIN_page);
}

void handleActuatorUp() {
  Serial.println("Moving up");
  server.send(200, "text/plain", "Moved up");

  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  delay(20000);
    
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  delay(1000);
}

void handleActuatorDown() {
  Serial.println("Moving down");
  server.send(200, "text/plain", "Moved down");

  digitalWrite(in2, HIGH);
  digitalWrite(in1, LOW);
  delay(20000);

  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  delay(1000);
}

void handleCalibrate() {
  Serial.println("Calibrating");
  server.send(200, "text/plain", "Calibrated");

  digitalWrite(in2, HIGH);
  digitalWrite(in1, LOW);
  delay(20000);

  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  delay(1000);
}

void setup() {
  Serial.begin(115200);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);

  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);

  WiFi.softAPConfig(local_ip, gateway, subnet);
  WiFi.softAP(ssid, password);
  
  Serial.println("SoftAP started");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/actuator/up", handleActuatorUp);
  server.on("/actuator/down", handleActuatorDown);
  server.on("/actuator/calibrate", handleCalibrate);

  server.begin();
  Serial.println("Web server started");
}

void loop() {
  server.handleClient();
}
