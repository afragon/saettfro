/*
   This code can not be runned by itself, it needs methods from
   the servo, fan, pump and the DHT sensor.
*/

#include "SimpleTimer.h"
SimpleTimer timer; // Defines the timer objecta

void setup() {
  timer.setInterval(600000, continuousCheck); //This method run continousCheck every 600000 milliseconds = 10 minutes.
}

void loop() {
  timer.run(); //start the timer
}

void continuousCheck() {
  const int tempLimit = 22; //Set the lower temperature limit
  const int highTempLimit = 24; //Set the higher temperature limit

  const int humidMinLimit = 45; //Set the minimum humidity limit
  const int humidMaxLimit = 65; //Set the maximum humidity limit

  float tempDiff = fTemp - tempLimit; //Calculates the lower temperature differance
  float highTempDiff = fTemp - highTempLimit; //Calculates the higher temperature differance
  float humidityMinDiff = fHumidity - humidMinLimit;  //Calculates the max humidity differance
  float humiditydMaxDiff = humidMaxLimit - fHumidity; //Calculates the min humidity differance

  Serial.print("Check Uptime (s): ");
  Serial.println(millis() / 1000); //Prints the time in seconds
  readSensor(); //Collects new data from sensors

  if ((fTemp < tempLimit)) { //If the temperature is lower than the limit the window close
    Serial.println("Low temp");
    Serial.println("Temp Diff: " + String(tempDiff));
    closeWindow();
  }
  if ((fTemp > highTempLimit) || (fHumidity > humidMaxLimit)) { //If the temperature och humidity is high the window is opened and the fan is started
    Serial.println("Temp or Humid high");
    Serial.println("Diff in temp: " + String(tempDiff));
    Serial.println("Diff in humidity: " + String(humiditydMaxDiff));
    openWindow();
    startFan();
  }
  else if ((fTemp > tempLimit)) { //If the temperature is high the window is opened and the fan is started
    Serial.println("High temp");
    Serial.println("Diff in temp: " + String(tempDiff));
    openWindow();
    startFan();
  }
  if (fHumidity < humidMinLimit) { //If the humidity is low the pump is started
    Serial.println("Low humid");
    Serial.println("Diff in humidity: " + String(humidityMinDiff));
    startPump();
  }
  waterCheck(); //Doing a watercheck to see if the Green House has been watered lately
  Serial.println("---");
  count++;
}

/*
    Stop different processes so they don't run simultaniusly, 1 = pump, 2 = ESP8266
*/
void pausProcess(int typeOfDevice) {
  if (typeOfDevice == 1) {
    if (fanStatus) {
      stopFan();
      continueFan = true;
    }
  }
  if (typeOfDevice == 2) {
    if (fanStatus) {
      stopFan();
      continueFan = true;
    }
    if (pumpStatus) {
      stopPump();
      continuePump = true;
    }
  }
}

/*
   If they have been paused they will continue again with this process
*/
void continueProcess() {
  if (continueFan) {
    startFan();
    continueFan = false;
  }
  if (continuePump) {
    startPump();
    continuePump = false;
  }
}

/*
   Doing a watercheck so the Green House has been watered lately
*/
void waterCheck() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastWateredMillis >= wateringInterval) {
    startPump();
  }
}

