#include <SoftwareSerial.h>
SoftwareSerial esp8266(2, 3); //RX, TX
#define IP "184.106.153.149" //thingspeak.com
String apiKey = "PIG49SYVRQ6S6K07"; //Get your personal key from thingspeak
#define DEBUG true

#include "DHT.h" //Includes the library
#define DHTPIN 13     //Digital pin that the sensor is connected to
#define DHTTYPE DHT22   //DHT 22 
DHT dht(DHTPIN, DHTTYPE);
//Read values from sensors
float fHumidity;
float fTemp;
int iHumidity;
int iTemp;

void setup() {
  esp8266.begin(9600); //ESP baudrate is 9600

  Serial.begin(9600); //Starting serial so we can print temp and humidity
  dht.begin(); //Starting the DHT sensor
}

void loop() {
  readSensor(); //Reads the values from the sensor
  //Prints the values to the serial monitor
  Serial.print("Humidity: ");
  Serial.print(fHumidity);
  Serial.println(" % \t");
  Serial.print("Temperature: ");
  Serial.print(fTemp);
  Serial.println(" *C ");
  delay(2000);
  updateThingSpeak();
  delay(40000);//wait 40 seconds so the data can be uploaded
}

/*
   This method sends data to ESP8266
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
   This method updates thingspeek with moist and temperature,
*/
void updateThingSpeak() {
  Serial.println("Update Thingspeek");
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
  getStr += "\r\n\r\n";

  // send data length
  cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());
  cmd += "\r\n\r\n";
  sendData(cmd, 4000, DEBUG);
  sendData(getStr, 4000, DEBUG);
  delay(6000);
}

/*
   Method to read data from the sensors
*/
void readSensor() {
  fHumidity = dht.readHumidity(); //Reads humidity data and saves it as float
  fTemp = dht.readTemperature(); //Reads temperature data and saves it as float
  iHumidity = dht.readHumidity(); //Reads humidity data and saves it as int
  iTemp = dht.readTemperature(); //Reads temperature data and saves it as int

  if (isnan(fHumidity) || isnan(fTemp) ) {   // Check if any reads failed
    Serial.println("Failed to read from DHT sensor!");
  }
}
