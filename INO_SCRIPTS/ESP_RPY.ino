#include "Wire.h"
#include <MPU6050_light.h>
#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "khaja-esp";
const char* password = "12345678";

IPAddress local_ip(192, 168, 10, 1);
IPAddress gateway(192, 168, 10, 1);
IPAddress subnet(255, 255, 255, 0);

WebServer server(80);

MPU6050 mpu(Wire);
unsigned long timer = 0;
int LED_BUILTIN = 2;

// ====== Handle sensor JSON response ======
void handleSensor() {
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

// ====== Setup ======
void setup() {
  Serial.begin(57600);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  Wire.begin();

  // Initialize MPU6050
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

  // Setup Wi-Fi
  WiFi.softAPConfig(local_ip, gateway, subnet);
  WiFi.softAP(ssid, password);
  Serial.println("SoftAP started");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  // Web routes
  server.on("/sensor", handleSensor);
  server.begin();
  Serial.println("Web server started");
}

// ====== Loop ======
void loop() {
  mpu.update();
  server.handleClient();

  if ((millis() - timer) > 100) {
    Serial.println(
      String(mpu.getAngleX(), 2) + ";" + 
      String(mpu.getAngleY(), 2) + ";" + 
      String(mpu.getAngleZ(), 2)
    );
    timer = millis();
  }
}
