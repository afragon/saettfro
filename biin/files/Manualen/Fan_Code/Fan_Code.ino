const int fanPin = 8;
boolean fanStatus = false;

void setup() {
  pinMode(fanPin, OUTPUT);
}
/*
 * Method to start the fan
 */
void startFan() {
  if (!fanStatus) {
    analogWrite(fanPin, 250);
    fanStatus = true;
  }
}
/*
 * Method to stop the fan
 */
void stopFan() {
  if (fanStatus) {
    digitalWrite(fanPin, LOW);
    fanStatus = false;
  }
}

void loop() {
  startFan();
  delay(5000);
  stopFan();
  delay(2000);
}

