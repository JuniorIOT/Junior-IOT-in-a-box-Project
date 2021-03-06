#include "Seeed_BME280.h"

//////////////////////////////////////////////////////////
//// GY-BMEP BME280
////////////////////////////////////////////

BME280 bme280;
float bme280_temperature;
float bme280_pressure;
float bme280_humidity;

void readBME(){  
  //get and print temperatures
  Serial.print(F("BME280 Temp: "));
  bme280_temperature = bme280.getTemperature();
  Serial.print(bme280_temperature);
  Serial.println(" deg C");  //The unit for  Celsius because original arduino don't support speical symbols
  
  //get and print atmospheric pressure data
  Serial.print(F("BME280 Pressure: "));
  bme280_pressure = bme280.getPressure();
  Serial.print(bme280_pressure);
  Serial.println(" Pa");

//  //get and print altitude data
//  Serial.print(F("BME280 Altitude: "));
  Serial.print(F("BME280 Humidity: "));
  bme280_humidity = bme280.getHumidity();
  Serial.print(bme280_humidity);
  Serial.println("%");
}
void put_BME_into_sendbuffer(){
  //readBME();

  // 
  uint16_t _temp = (bme280_temperature + 100 )*100 ;
  myLoraWanData[BME280_sendbufferStartByte + 0] = _temp >> 8;
  myLoraWanData[BME280_sendbufferStartByte + 1] = _temp;
  
  uint16_t _hum = (bme280_humidity) * 100 ;
  myLoraWanData[BME280_sendbufferStartByte + 2] = _hum >> 8;
  myLoraWanData[BME280_sendbufferStartByte + 3] = _hum;
  
  uint16_t _p = (bme280_pressure)/10 ;
  myLoraWanData[BME280_sendbufferStartByte + 4] = _p >> 8;
  myLoraWanData[BME280_sendbufferStartByte + 5] = _p;  
}

//------------------------

void BME280_measure () {  
  readBME();  
  put_BME_into_sendbuffer();
}

void BME280_init() {  // see SEEDSTUDIO example
  Serial.print(F("BME280 init. t=")); Serial.println(millis());

  //  can use first 4 pins are soldered to first 4 pins on lora32u4
  //   Vin to pin 6  // optional Vin
  //   GND to pin 5  // optional GND
  //   SCL to SCL = 3
  //   SDA to SDA = 2
  pinMode(6, OUTPUT);
  pinMode(5, OUTPUT);
  digitalWrite(6, HIGH);
  digitalWrite(5, LOW);  
  delay(100);  // need to allow boot time for the device
 
  if(!bme280.init()){
    Serial.println("BME280 device error or not found");
  }
  
  // as part of init, do one measurement to load values into send buffer
  BME280_measure();
}
