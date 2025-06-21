#include <WiFi.h>
#include <WebServer.h>
#include "Wire.h"
#include <MPU6050_light.h>

const char* ssid = "khaja-esp";
const char* password = "12345678";

IPAddress local_ip(192, 168, 10, 1);
IPAddress gateway(192, 168, 10, 1);
IPAddress subnet(255, 255, 255, 0);

WebServer server(80);

int in1 = 13;
int in2 = 12;
MPU6050 mpu(Wire);
unsigned long timer = 0;
int LED_BUILTIN = 2;

// Webpage HTML
const char MAIN_page[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Project Nautilius</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial; text-align: center; margin-top: 50px; background: #111; color: white; }
    button { padding: 20px 40px; font-size: 20px; margin: 20px; border: none; border-radius: 10px; }
    .up { background-color: black; color: white; }
    .down { background-color: black; color: white; }
    .calibrate { background-color: red; color: white; }
    .stop { background-color: red; color: white; }
    #status { margin-top: 30px; font-size: 24px; color: yellow; }
  </style>
</head>
<body>
  <h1>Actuator Control</h1>
  <button class="up" onclick="sendCommand('/actuator/up')">Up</button>
  <button class="down" onclick="sendCommand('/actuator/down')">Down</button>
  <button class="stop" onclick="sendCommand('/actuator/stop')">STOP</button>
  <button class="stop" onclick="sendCommand('/actuator/dive')">DIVE</button>
  <button class="calibrate" onclick="sendCommand('/actuator/calibrate')">Calibrate</button>
  <div id="status">Status: Idle</div>

  <script>
    function sendCommand(path) {
      fetch(path)
        .then(response => response.text())
        .then(data => {
          document.getElementById('status').innerText = 'Status: ' + data;
        })
        .catch(error => {
          document.getElementById('status').innerText = 'Status: Error';
        });
    }
  </script>
</body>
</html>
)rawliteral";

// Route handlers
void handleRoot() {
  server.send(200, "text/html", MAIN_page);
}

void handleActuatorUp() {
  Serial.println("Moving up");
  server.send(200, "text/plain", "Moving up");

  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
}

void handleActuatorDown() {
  Serial.println("Moving down");
  server.send(200, "text/plain", "Moving down");

  digitalWrite(in2, HIGH);
  digitalWrite(in1, LOW);
}

void handleActuatorCalibrate() {
  Serial.println("Calibrating");
  server.send(200, "text/plain", "Calibrating");

  digitalWrite(in2, HIGH);
  digitalWrite(in1, LOW);
}

void handleActuatorStop() {
  Serial.println("Stopping");
  server.send(200, "text/plain", "Stopping");

  digitalWrite(in2, LOW);
  digitalWrite(in1, LOW);
}

void handleActuatorDive() {
  Serial.println("DIVING T - 60 Seconds");
  server.send(200, "text/plain", "DIVING T - 60 Seconds");

  delay(20000);
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);

  delay(30000);

  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  delay(100);

  digitalWrite(in2, HIGH);
  digitalWrite(in1, LOW);
  delay(30000);

  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  delay(100);
}

void handleSensor() {
  digitalWrite(LED_BUILTIN, HIGH);
  mpu.update();
  float ax = mpu.getAngleX();
  float ay = mpu.getAngleY();
  float az = mpu.getAngleZ();

  String json = "{";
  json += "\"x\":" + String(ax, 2) + ",";
  json += "\"y\":" + String(ay, 2) + ",";
  json += "\"z\":" + String(az, 2);
  json += "}";

  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);

  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);

  WiFi.softAPConfig(local_ip, gateway, subnet);
  WiFi.softAP(ssid, password);
  
  Serial.println("SoftAP started");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  Wire.begin();
  byte status = mpu.begin();
  Serial.print(F("MPU6050 status: "));
  Serial.println(status);
  while (status != 0) {
    delay(1000);  // Retry if connection failed
  }
  Serial.println(F("Calculating offsets, do not move MPU6050"));
  delay(1000);
  mpu.calcOffsets();  // Calibrate
  Serial.println("Done!");

  server.on("/", handleRoot);
  server.on("/actuator/up", handleActuatorUp);
  server.on("/actuator/down", handleActuatorDown);
  server.on("/actuator/stop", handleActuatorStop);
  server.on("/actuator/dive", handleActuatorDive);
  server.on("/actuator/calibrate", handleActuatorCalibrate);
  server.on("/sensor", handleSensor);

  server.begin();
  Serial.println("Web server started");
}

void loop() {
  server.handleClient();
}
