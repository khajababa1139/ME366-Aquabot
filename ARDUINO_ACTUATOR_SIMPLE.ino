// Motor control pins
const int in1 = 8;
const int in2 = 9;

void setup() {
pinMode(in1, OUTPUT);
pinMode(in2, OUTPUT);
}

void loop() {
// Rotate motor in one direction
digitalWrite(in1, HIGH);
digitalWrite(in2, LOW);
delay(20000); // 3 seconds

// Stop motor
digitalWrite(in1, LOW);
digitalWrite(in2, LOW);
delay(1000); // 1 second pause

// Rotate motor in opposite direction
digitalWrite(in1, LOW);
digitalWrite(in2, HIGH);
delay(20000); // 3 seconds

// Stop motor
digitalWrite(in1, LOW);
digitalWrite(in2, LOW);
delay(1000); // 1 second pause
}
