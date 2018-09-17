
// test for BME280 

#include <Wire.h> //I2C Arduino Library
#include "Seeed_BME280.h"

//////////////////////////////////////////////////////////
//// GY-BMEP BME280
////////////////////////////////////////////

BME280 bme280;

void readBME(){
  
  //get and print temperatures
  Serial.print(F("BME280 Temp: "));
  float bme280_temperature = bme280.getTemperature();
  Serial.print(bme280_temperature);
  Serial.println(" deg C");  //The unit for  Celsius because original arduino don't support speical symbols
  
  //get and print atmospheric pressure data
  Serial.print(F("BME280 Pressure: "));
  float bme280_pressure = bme280.getPressure();
  Serial.print(bme280_pressure);
  Serial.println(" Pa");

//  //get and print altitude data
//  Serial.print(F("BME280 Altitude: "));
  Serial.print(F("BME280 Humidity: "));
  float bme280_humidity = bme280.getHumidity();
  Serial.print(bme280_humidity);
  Serial.println("%");
 
  Serial.println();

}

void BME280_init() {  // see SEEDSTUDIO example
  Serial.print(F("BME280 init. ")); 
  //   Vin to 3V3
  //   GND to gnd
  //   SCL to SCL = 3
  //   SDA to SDA = 2
 
  if(!bme280.init()){
    Serial.println("BME280 device error or not found");
  }

}

void setup() {
  Serial.begin(115200);
  delay(500);
  
  Serial.println(F("\nStarting BME280 test.")); 
  BME280_init();
}

void loop() {
  readBME();
  delay(1000);
}

