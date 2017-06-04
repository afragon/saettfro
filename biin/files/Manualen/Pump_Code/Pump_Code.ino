const int pumpPin = 6;
unsigned long lastWateredMillis;

void setup() {
  pinMode(pumpPin, OUTPUT);
}
/*
 * Method to start the pump
 */
void startPump() {
  digitalWrite(pumpPin, HIGH);
}
/*
 * Method to stop the pump
 */
void stopPump() {
  digitalWrite(pumpPin, LOW);
  lastWateredMillis = millis();
}

void loop() {
  startPump();
  delay(10000);
  stopPump();
  delay(10000);
}
