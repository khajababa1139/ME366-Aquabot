#include <SoftwareSerial.h>
#include <WiFi.h>
#include <WebServer.h>
#include "Wire.h"
#include <MPU6050_light.h>
#include <TinyGPS++.h>

#define DOUT_PIN 19  // Data output pin from HX710B
#define SCK_PIN 18   // Clock pin for HX710B

#define GPS_TX_PIN 2
#define GPS_RX_PIN 4

// WiFi settings
const char* ssid = "khaja-esp";
const char* password = "estyak1139";

IPAddress local_ip(192, 168, 10, 1);
IPAddress gateway(192, 168, 10, 1);
IPAddress subnet(255, 255, 255, 0);

// HC-12 on GPIO16 (RX), GPIO17 (TX) for ESP32
SoftwareSerial HC12(16, 17); // RX, TX
MPU6050 mpu(Wire);
unsigned long timer = 0;

TinyGPSPlus gps;
unsigned long timestamp;
float lat = 0;
float lng = 0;
float alt = 0;
float ground_speed = 0;

// Motor and actuator pins
const int ledPin = 2;
const int in1 = 12;
const int in2 = 13;
const int in3 = 26;
const int in4 = 27;
const int in5 = 32;
const int in6 = 33;

// Status variables
String actuatorState = "Stopped";
String leftMotorState = "Stopped";
String rightMotorState = "Stopped";

// Web server on port 80
WebServer server(80);

// HTML page in PROGMEM
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta http-equiv="refresh" content="1">
  <title>ESP32 Motor Status</title>
  <style>
    body { font-family: Arial; background: #121212; color: #eee; text-align: center; }
    .status-box { margin-top: 50px; padding: 20px; border: 1px solid #333; border-radius: 8px; display: inline-block; background: #1e1e1e; }
    h2 { margin-bottom: 10px; }
    p { font-size: 18px; margin: 5px 0; }
  </style>
</head>
<body>
  <div class="status-box">
    <h2>ESP32 Motor & Actuator Status</h2>
    <p><strong>Actuator:</strong> %ACTUATOR%</p>
    <p><strong>Left Motor:</strong> %LEFT%</p>
    <p><strong>Right Motor:</strong> %RIGHT%</p>
  </div>
</body>
</html>
)rawliteral";

// Replace placeholders in HTML with real-time data
String processor(const String& var) {
  if (var == "ACTUATOR") return actuatorState;
  if (var == "LEFT") return leftMotorState;
  if (var == "RIGHT") return rightMotorState;
  return String();
}

void setup() {
  Serial.begin(115200);
  HC12.begin(9600);

  // Configure and start SoftAP
  WiFi.softAPConfig(local_ip, gateway, subnet);
  WiFi.softAP(ssid, password);

  Serial.println("WiFi SoftAP started ...");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  // Pin setup
  pinMode(ledPin, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(in5, OUTPUT);
  pinMode(in6, OUTPUT);

  digitalWrite(ledPin, LOW);
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
  digitalWrite(in5, LOW);
  digitalWrite(in6, LOW);
  Serial.println("Pins Initialized ...");

  // Initialize MPU6050
  Wire.begin();
  byte status = mpu.begin();
  Serial.print(F("MPU6050 status: "));
  Serial.println(status);
  if (status != 0) {
    delay(1000);  // Retry if connection failed
  }

  Serial.println(F("Calculating offsets, do not move MPU6050"));
  delay(1000);
  mpu.calcOffsets();  // Calibrate
  Serial.println("Done!");

  pinMode(DOUT_PIN, INPUT);  // Set DOUT as input
  pinMode(SCK_PIN, OUTPUT); // Set SCK as output
  Serial.println("HX710B Pressure Sensor Initialized ...");

  Serial1.begin(9600, SERIAL_8N1, GPS_TX_PIN, GPS_RX_PIN);
  Serial.println("GPS Initialized ...");

  // Setup web server
  server.on("/", []() {
    String html = index_html;
    html.replace("%ACTUATOR%", actuatorState);
    html.replace("%LEFT%", leftMotorState);
    html.replace("%RIGHT%", rightMotorState);
    server.send(200, "text/html", html);
  });
  server.on("/MPU", handleMPU);
  server.on("/HX", handleHX);
  server.on("/GPS", handleGPS);

  server.begin();
  Serial.println("System Initialization Successful");
  Serial.println("Web server started");
}

void handleMPU() {
  float ax = mpu.getAngleX();
  float ay = mpu.getAngleY();
  float az = mpu.getAngleZ();

  // Build JSON response
  String json = "{";
  json += "\"x\":" + String(ax, 2) + ",";
  json += "\"y\":" + String(ay, 2) + ",";
  json += "\"z\":" + String(az, 2);
  json += "}";

  // Send to HTTP client
  server.send(200, "application/json", json);

  // Optional: Print to Serial for debugging
  Serial.print("MPU: ");
  Serial.print(ax, 2);
  Serial.print(", ");
  Serial.print(ay, 2);
  Serial.print(", ");
  Serial.println(az, 2);
}

long readPressure() {
  long data = 0;  // Variable to store pressure data
  while (digitalRead(DOUT_PIN) == HIGH); // Wait for DOUT to go LOW

  for (int i = 0; i < 24; i++) {
    digitalWrite(SCK_PIN, HIGH); // Generate clock pulse
    data = (data << 1) | digitalRead(DOUT_PIN); // Read data bit
    digitalWrite(SCK_PIN, LOW);  // End clock pulse
  }

  // HX710B sends 24-bit data; shift to align with signed 32-bit integer
  data = data ^ 0x800000; // Convert to signed value
  return data;
}

void handleHX() {
  long pressure = readPressure();

  String json = "{";
  json += "\"pressure\":" + String(pressure);
  json += "}";

  server.send(200, "application/json", json);
  Serial.print("Pressure Data: ");
  Serial.println(pressure);
}

void handleGPS() {
  // Read and encode available GPS data
  if (Serial1.available() > 0) {
    gps.encode(Serial1.read());
  }

  String json = "{";

  // Check if GPS location is valid
  if (gps.location.isValid()) {
    Serial.println("GPS Data Valid");
    lat = gps.location.lat();
    lng = gps.location.lng();
    alt = gps.altitude.meters();
    ground_speed = gps.speed.kmph();

    // Construct JSON with GPS values
    json += "\"lat\":" + String(lat, 6) + ",";
    json += "\"lng\":" + String(lng, 6) + ",";
    json += "\"alt\":" + String(alt, 2) + ",";
    json += "\"speed\":" + String(ground_speed, 2);
  } else {
    // Return error message in JSON if data invalid
    json += "\"error\":\"Invalid GPS data\"";
    Serial.println("GPS Data NOT Valid");
  }

  json += "}";

  // Send response to client
  server.send(200, "application/json", json);
}

void moveActuatorUp() {
  Serial.println("Actuator Motor Up");
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  actuatorState = "Up";
}

void moveActuatorDown() {
  Serial.println("Actuator Motor Down");
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  actuatorState = "Down";
}

void stop() {
  Serial.println("All stop");
  digitalWrite(in1, LOW); digitalWrite(in2, LOW);
  digitalWrite(in3, LOW); digitalWrite(in4, LOW);
  digitalWrite(in5, LOW); digitalWrite(in6, LOW);

  actuatorState = "Stopped";
  leftMotorState = "Stopped";
  rightMotorState = "Stopped";
}

void rightMotorUp() {
  Serial.println("Right Motor Up");
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  rightMotorState = "Up";
}

void rightMotorDown() {
  Serial.println("Right Motor Down");
  digitalWrite(in4, HIGH);
  digitalWrite(in3, LOW);
  rightMotorState = "Down";
}

void leftMotorUp() {
  Serial.println("Left Motor Up");
  digitalWrite(in5, HIGH);
  digitalWrite(in6, LOW);
  leftMotorState = "Up";
}

void leftMotorDown() {
  Serial.println("Left Motor Down");
  digitalWrite(in6, HIGH);
  digitalWrite(in5, LOW);
  leftMotorState = "Down";
}

void loop() {
  mpu.update();
  server.handleClient();

  if (HC12.available()) {
    String received = HC12.readStringUntil('\n');
    received.trim();

    if (received.length() >= 7) {
      if (received[0] == '1') {
        moveActuatorUp();
      } else if (received[1] == '1') {
        moveActuatorDown();
      } else if (received[2] == '1') {
        stop();
      } else if (received[3] == '1') {
        leftMotorUp();
      } else if (received[4] == '1') {
        leftMotorDown();
      } else if (received[5] == '1') {
        rightMotorUp();
      } else if (received[6] == '1') {
        rightMotorDown();
      }
    }
  }
}
