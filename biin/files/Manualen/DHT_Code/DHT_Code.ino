#include "DHT.h" //Includes the library
#define DHTPIN 13     // Digital pin that the sensor is connected to
#define DHTTYPE DHT22   // DHT 22 
DHT dht(DHTPIN, DHTTYPE);
//Read values from sensors
float fHumidity;
float fTemp;

void setup() {
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
  delay(2000); //2 seconds until new value
}

/*
 * Method to read data from the sensors
 */
void readSensor() {
  fHumidity = dht.readHumidity(); //Reads humidity data from sensor
  fTemp = dht.readTemperature(); //Reads temperature data from sensor
  // Check if any reads failed and exit early (to try again)
  if (isnan(fHumidity) || isnan(fTemp) ) {
    Serial.println("Failed to read from DHT sensor!");
  }
}
