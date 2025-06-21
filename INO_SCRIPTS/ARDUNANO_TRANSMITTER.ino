#include <SoftwareSerial.h>

SoftwareSerial HC12(9, 10); // RX, TX (to HC-12)

// Define button pins
const int buttonPins[] = {3, 2, 4, 5, 7, 6, 8};
const int numButtons = sizeof(buttonPins) / sizeof(buttonPins[0]);

int buttonStates[numButtons];

void setup() {
  Serial.begin(9600);
  HC12.begin(9600); // HC-12 default baud rate

  for (int i = 0; i < numButtons; i++) {
    pinMode(buttonPins[i], INPUT);
  }
}

void loop() {
  // Read button states and send
  String dataToSend = "";

  for (int i = 0; i < numButtons; i++) {
    buttonStates[i] = digitalRead(buttonPins[i]);
    dataToSend += String(buttonStates[i]);

    if (i < numButtons - 1) dataToSend += "";
  }

  HC12.println(dataToSend); // Send button states via HC-12
  Serial.println(dataToSend);

  delay(10); // Short delay to prevent flooding
}