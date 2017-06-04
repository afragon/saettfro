// Project: Så ett frö
// Last updated: 2016-08-15

#include "SimpleTimer.h"

SimpleTimer timer; //Defines the timer object

//Definitions for temp and moist sensor
#include "DHT.h" //Includes the library
#define DHTPIN 13     //Digital pin that the sensor is connected to
#define DHTTYPE DHT22   //DHT22
DHT dht(DHTPIN, DHTTYPE);

//Wifi module
#define DEBUG true
int count = 0;

#include <SoftwareSerial.h> // Includes the library
SoftwareSerial esp8266(2, 3); // RX, TX
#define IP "184.106.153.149" // thingspeak.com
String apiKey = "PIG49SYVRQ6S6K07"; // Get your personal key from thingspeak

//Fan
const int fanPin = 8;
boolean fanStatus = false;
boolean continueFan = false;

//Pump
const int pumpPin = 6;
boolean pumpStatus = false;
boolean continuePump = false;
boolean thingSpeakWatered = false;

//Servo
#include <Servo.h>
Servo myservo;

//Servo variables and constants
const int windowDegreeOpen = 140; //Degrees needed for the window to be opened
const int windowDegreeClosed = 90; //Degrees needed for the window to be closed
int servoPosition = 0;   //Variable to store the servo position
boolean windowOpen = false;

//Constants for the logic
const long timerFan = 180000; // How long the fan is on, in milliseconds
const long timerPump = 20000; //How long the pump is on, in milliseconds
const long timerWatering = 18000000; //Minimum time before next watering (for sensors)
boolean recentlyWatered = false; //Boolean for the sensors to know if its okey to water
unsigned long lastWateredMillis;

//Read values from sensors
float fHumidity;
float fTemp;
int iHumidity;
int iTemp;

void setup() {
  //ESP8266
  Serial.begin(9600);
  Serial.println("Running Setup");
  esp8266.begin(9600); //Creates the serial monitor with correct Baud Rate

  timer.setInterval(600000, continuousCheck); //This method run continousCheck in the specifiedd interval

  //DHT sensor
  dht.begin();

  //Servo
  topos(windowDegreeClosed);

  //Fan
  pinMode(fanPin, OUTPUT);

  //Pump
  pinMode(pumpPin, OUTPUT);

  //Initialise the sensor values
  readSensor();
  delay(5000);
  continuousCheck();
}

void loop() {
  timer.run(); //Start the timer
}
/*
   Doing a watercheck, checking so the Green House has been watered lately
*/
void waterCheck() {
  unsigned long currentMillis = millis();
  Serial.println("Doing waterCheck");
  if (currentMillis - lastWateredMillis >= timerWatering) {
    // Serial.println("Need water");
    startPump();
  }
  else {
    //Serial.println("Don't need water");
  }
}

void continuousCheck() {
  const int tempLimit = 20; //Set the lower temperature limit
  const int highTempLimit = 23; //Set the higher temperature limit

  const int humidMinLimit = 60; //Set the minimum humidity limit 45
  const int humidMaxLimit = 75; //Set the maximum humidity limit 65

  float tempDiff = fTemp - tempLimit; //Calculates the lower temperature differance
  float highTempDiff = fTemp - highTempLimit; //Calculates the higher temperature differance
  float humidityMinDiff = fHumidity - humidMinLimit;  //Calculates the max humidity differance
  float humiditydMaxDiff = humidMaxLimit - fHumidity; //Calculates the min humidity differance

  Serial.print("Check Uptime (s): ");
  Serial.println(millis() / 1000); //Prints the time in seconds
  readSensor(); //Collects new data from sensors
  waterCheck(); //Starting the method waterCheck
  
  if (fHumidity < humidMinLimit) { //If the humidity is low the pump is started
    Serial.println("Low humid");
    Serial.println("Diff in humidity: " + String(humidityMinDiff));
    startPump();
  }
  else if ((fTemp > highTempLimit) || (fHumidity > humidMaxLimit)) { //If the temperature och humidity is high the window is opened and the fan is started
    Serial.println("Temp or Humid high");
    Serial.println("Diff in temp: " + String(tempDiff));
    Serial.println("Diff in humidity: " + String(humiditydMaxDiff));
    openWindow();
    startFan();
  }
  else if ((fTemp < tempLimit)) { //If the temperature is high the window is opened and the fan is started
    Serial.println("Low temp");
    Serial.println("Temp Diff: " + String(tempDiff));
    closeWindow();
  }
  else if ((fTemp > tempLimit)) { //If the temperature is higher than the limit the window open
    Serial.println("High temp");
    Serial.println("Diff in temp: " + String(tempDiff));
    openWindow();
  }
  else if ((fTemp < tempLimit)) { //If the temperature is high the window is opened and the fan is started
    Serial.println("Low temp");
    Serial.println("Temp Diff: " + String(tempDiff));
    closeWindow();
  }
  
  updateThingSpeak();
  Serial.println("---");
}
/*
   Method to start the pump
*/
void startPump() {
  if (recentlyWatered) {
    Serial.println("Recently watered");
  }
  else {
    Serial.println("Starting the pump");
    //pausProcess(1);
    //timer.setTimeout(timerPump, stopPump);
    digitalWrite(pumpPin, HIGH);
    lastWateredMillis = millis();
    pumpStatus = true;
    delay(timerPump + 2000);
    stopPump();
    thingSpeakWatered = true;
  }
}
/*
   Method to stop the pump
*/
void stopPump() {
  Serial.println("Stoping the pump");
  digitalWrite(pumpPin, LOW);
  recentlyWatered = true;
  timer.setTimeout(timerWatering, wateringReady);
  pumpStatus = false;
  continueProcess();
}

void wateringReady() {
  recentlyWatered = false;
}

void stopFan() {
  Serial.println("Stopping the fan");
  digitalWrite(fanPin, LOW);
  fanStatus = false;
}

void startFan() {
  Serial.println("StartFan");
  if (fanStatus) {
    // Serial.println("The fan is running");
  }
  else {
    Serial.println("Starting the fan, nice!");
    timer.setTimeout(timerFan, stopFan);
    analogWrite(fanPin, 250);
    fanStatus = true;
  }
}

/*
   This method is used to sends data to ESP8266
*/
String sendData(String command, const int timeout, boolean debug)
{
  String response = "";
  esp8266.print(command); //Send the read character to the esp8266
  long int time = millis();
  while ( (time + timeout) > millis())
  {
    while (esp8266.available())
    {
      // The esp has data so display its output to the serial window
      char c = esp8266.read(); // read the next character.
      response += c;
    }
  }
  if (debug)
  {
    Serial.print(response);
  }
  return response;
}

/*
   Send data to thingspeaks, for tweeting
*/
void tweet() {
  // Serial.println("It is time for a tweet!");
  pausProcess(2);
  delay(2000);

  sendData("AT+RST\r\n", 2000, DEBUG); // Restart the ESP
  sendData("AT\r\n", 8000, DEBUG); //Check status of ESP

  String tweetmessage = "Today it is: ";
  tweetmessage += iTemp;
  tweetmessage += " degrees celsius";
  tweetmessage += " and the humidity level is:";
  tweetmessage += iHumidity;
  //tweetmessage += ", I'm löving it!";
  //String tweetmessage = "Nu testar vi igen!";
  // TCP connection
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += "api.thingspeak.com"; // api.thingspeak.com
  cmd += "\",80";
  cmd += "\r\n";

  sendData(cmd, 4000, DEBUG);
  delay(5000);

  // prepare GET string
  String getStri = "GET /apps/thingtweet/1/statuses/update?api_key=XHGHL6NAZLREELGP&status=";
  getStri += tweetmessage;
  getStri += "\r\n\r\n"; //la till en extra rn

  String cmd2 = "AT+CIPSEND=";
  cmd2 += String(getStri.length());
  cmd2 += "\r\n\r\n";

  sendData(cmd2, 4000, DEBUG);
  sendData(getStri, 4000, DEBUG);

  cmd2 = " ";
  sendData(cmd2, 1000, DEBUG);
  sendData(cmd2, 1000, DEBUG);
  delay(10000);

  // sendData("AT+CIPCLOSE\r\n", 1000, DEBUG);
  // delay (5000);
  continueProcess();
}

/*
   This method updates thingspeek with moist and temperature,
*/
void updateThingSpeak() {
  // Serial.println("Update Thingspeek");
  pausProcess(2);

  delay(2000);
  sendData("AT+RST\r\n", 2000, DEBUG); // Restart the ESP
  sendData("AT\r\n", 8000, DEBUG); //Check status of ESP

  // TCP connection
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += IP;
  cmd += "\",80";
  cmd += "\r\n";

  sendData(cmd, 4000, DEBUG);
  delay(5000);

  // prepare GET string
  String getStr = "GET /update?api_key=";
  getStr += apiKey;
  getStr += "&field1=";
  getStr += String(iTemp);
  getStr += "&field2=";
  getStr += String(iHumidity);

  if (thingSpeakWatered) {
    getStr += "&field3=";
    getStr += "1";
    thingSpeakWatered = false;
  }
  else {
    getStr += "&field3=";
    getStr += "0";
  }
  if (windowOpen) {
    getStr += "&field4=";
    getStr += "1";
  }
  else {
    getStr += "&field4=";
    getStr += "0";
  }
  if (continueFan) {
    getStr += "&field5=";
    getStr += "1";
  }
  else {
    getStr += "&field5=";
    getStr += "0";
  }
  getStr += "\r\n\r\n";

  // send data length
  cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());
  cmd += "\r\n\r\n";
  sendData(cmd, 4000, DEBUG);
  sendData(getStr, 4000, DEBUG);
  delay(6000);
  continueProcess();
}

/*
   This method reads the tempature and humidity from the dht sensor
*/
void readSensor() {
  fHumidity = dht.readHumidity(); //Reads humidity data and saves it as float
  fTemp = dht.readTemperature(); //Reads temperature data and saves it as float
  iHumidity = dht.readHumidity(); //Reads humidity data and saves it as int
  iTemp = dht.readTemperature(); //Reads temperature data and saves it as int

  if (isnan(fHumidity) || isnan(fTemp) ) { //Check if any reads failed
    Serial.println("Failed to read from DHT sensor!");
  }

  Serial.print("Humidity: ");
  Serial.print(fHumidity);
  Serial.println(" %\t");
  Serial.print("Temperature: ");
  Serial.print(fTemp);
  Serial.println(" *C ");
}
/**
  This method opens the window
*/
void openWindow() {
  if (!windowOpen) {
    Serial.println("opening");
    topos(windowDegreeOpen);
    windowOpen = true;
  }
}
/*
   This metod closes the window
*/
void closeWindow() {
  if (windowOpen) {
    topos(windowDegreeClosed);
    windowOpen = false;
  }
}
/*
   This method turns the servo up or down
*/
void topos(int gotopos) {
  myservo.attach(9); //Makes the servo ready for use
  delay(3000);
  if (servoPosition <= gotopos) {
    for (servoPosition; servoPosition <= gotopos; servoPosition += 1) { //Turns in steps of 1 degree
      myservo.write(servoPosition); //Tell servo to go to position in variable 'servoPosition'
      delay(100); //Adjusts the speed of the servo when it moves into position
    }
  }
  else {
    for (servoPosition; servoPosition >= gotopos; servoPosition -= 1) { //Turns in steps of 1 degree
      myservo.write(servoPosition); //Tell servo to go to position in variable 'servoPosition'
      delay(100); //Adjusts the speed of the servo when it moves into position
    }
  }
  myservo.detach(); //Detaches servo so it won’t respond to current
}

/*
    Stop different processes so they don't run simultaniusly, 1 = pump, 2 = ESP8266
*/
void pausProcess(int typeOfDevice) {
  Serial.println("paus process kors");
  if (typeOfDevice == 1) {
    if (fanStatus) {
      stopFan();
      continueFan = true;
      Serial.println("paus process FAN1");
    }
  }
  if (typeOfDevice == 2) {
    if (fanStatus) {
      stopFan();
      continueFan = true;
      Serial.println("paus process FAN2");
    }
    if (pumpStatus) {
      stopPump();
      continuePump = true;
      Serial.println("paus process PUMP");
    }
  }
}

/*
   If the methods have been paused they will continue again with this process
*/
void continueProcess() {
  Serial.println("Continue process kors");
  if (continueFan) {
    Serial.println("Continue process FAN");
    startFan();
    continueFan = false;
  }
}

