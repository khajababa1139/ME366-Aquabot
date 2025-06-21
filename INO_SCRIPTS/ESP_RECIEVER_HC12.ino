#include <SoftwareSerial.h>

// HC-12 on GPIO16 (RX), GPIO17 (TX) for ESP32
SoftwareSerial HC12(16, 17); // RX, TX

const int ledPin = 2;   // GPIO2 (LED)
const int in1 = 13;     // Actuator control pin
const int in2 = 14;     // Actuator control pin

void setup() {
  Serial.begin(9600);       
  HC12.begin(9600);         

  pinMode(ledPin, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);

  digitalWrite(ledPin, LOW);
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
}

void moveActuatorUp() {
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
}

void moveActuatorDown() { 
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
}

void stopActuator() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
}

void diveActuator() {
  moveActuatorUp();
  delay(30000);
  moveActuatorDown();
  delay(30000);
  stopActuator();
}

void loop() {
  if (HC12.available()) {
    String received = HC12.readStringUntil('\n');
    received.trim(); 
    Serial.println(received);

    if (received.length() >= 7) {
      if (received[0] == '1') {

        moveActuatorUp();
      } else if (received[1] == '1') {

        moveActuatorDown();
      } else if (received[2] == '1') {
  
        stopActuator();
      } else if (received[3] == '1') {

        diveActuator();
      } else if(received[4] == '1') {

        digitalWrite(ledPin, HIGH);
      } else if(received[5] == '1') {

        digitalWrite(ledPin, LOW);
      }
    }
  }
}
