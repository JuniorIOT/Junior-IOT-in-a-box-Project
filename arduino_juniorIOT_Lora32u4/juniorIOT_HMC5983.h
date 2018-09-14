


#define HMC5983_selector_for_X_byte0 0x03  
#define HMC5983_selector_for_X_byte1 0x04  
#define HMC5983_selector_for_Z_byte0 0x05  
#define HMC5983_selector_for_Z_byte1 0x06  
#define HMC5983_selector_for_Y_byte0 0x07  
#define HMC5983_selector_for_Y_byte1 0x08  
int HMC5983_X_byte0, HMC5983_X_byte1, HMC5983_X_value;
int HMC5983_Y_byte0, HMC5983_Y_byte1, HMC5983_Y_value;
int HMC5983_Z_byte0, HMC5983_Z_byte1, HMC5983_Z_value;
float X_milliGauss,Y_milliGauss,Z_milliGauss;
float heading, headingDegrees, headingFiltered, geo_magnetic_declination_deg;
#define IIC_HMC5983_address 0x1E //I2C 7bit address of HMC5883


//////////////////////////////////////////////////////////
//// Compass HMC5983
////////////////////////////////////////////
void Compass_init() {
  Serial.print(F("Compass init. t=")); Serial.println(millis());
  
  //  can use first 4 pins are soldered to first 4 pins on lora32u4
  //   Vin to pin 6  // optional Vin
  //   GND to pin 5  // optional GND
  //   SCL to SCL = 3
  //   SDA to SDA = 2
  pinMode(6, OUTPUT);
  pinMode(5, OUTPUT);
  digitalWrite(6, HIGH);
  digitalWrite(5, LOW);
  
  Wire.begin();
  delay(100);
  
  Wire.beginTransmission(IIC_HMC5983_address); 
  Wire.write(0x02); // Select mode register
  Wire.write(0x00); // Continuous measurement mode
  Wire.endTransmission();
}

long readCompass() {
  //---- X-Axis
  Wire.beginTransmission(IIC_HMC5983_address); // transmit to device
  Wire.write(HMC5983_selector_for_X_byte1);
  Wire.endTransmission();
  Wire.requestFrom(IIC_HMC5983_address,1); 
  if(Wire.available()<=1)   
  {
    HMC5983_X_byte0 = Wire.read();
  }
  Wire.beginTransmission(IIC_HMC5983_address); // transmit to device
  Wire.write(HMC5983_selector_for_X_byte0);
  Wire.endTransmission();
  Wire.requestFrom(IIC_HMC5983_address,1); 
  if(Wire.available()<=1)   
  {
    HMC5983_X_byte1 = Wire.read();
  }
  //---- Y-Axis
  Wire.beginTransmission(IIC_HMC5983_address); // transmit to device
  Wire.write(HMC5983_selector_for_Y_byte1);
  Wire.endTransmission();
  Wire.requestFrom(IIC_HMC5983_address,1); 
  if(Wire.available()<=1)   
  {
    HMC5983_Y_byte0 = Wire.read();
  }
  Wire.beginTransmission(IIC_HMC5983_address); // transmit to device
  Wire.write(HMC5983_selector_for_Y_byte0);
  Wire.endTransmission();
  Wire.requestFrom(IIC_HMC5983_address,1); 
  if(Wire.available()<=1)   
  {
    HMC5983_Y_byte1 = Wire.read();
  }
  
  //---- Z-Axis
  Wire.beginTransmission(IIC_HMC5983_address); // transmit to device
  Wire.write(HMC5983_selector_for_Z_byte1);
  Wire.endTransmission();
  Wire.requestFrom(IIC_HMC5983_address,1); 
  if(Wire.available()<=1)   
  {
    HMC5983_Z_byte0 = Wire.read();
  }
  Wire.beginTransmission(IIC_HMC5983_address); // transmit to device
  Wire.write(HMC5983_selector_for_Z_byte0);
  Wire.endTransmission();
  Wire.requestFrom(IIC_HMC5983_address,1); 
  if(Wire.available()<=1)   
  {
    HMC5983_Z_byte1 = Wire.read();
  }
  
  //---- X-Axis
  HMC5983_X_byte1 = HMC5983_X_byte1 << 8;
  HMC5983_X_value = HMC5983_X_byte0 + HMC5983_X_byte1; // Raw data
  // From the datasheet: 0.92 mG/digit
  X_milliGauss = HMC5983_X_value * 0.00092; // Gauss unit
  //* Earth magnetic field ranges from 0.25 to 0.65 Gauss, so these are the values that we need to get approximately.
  
  //---- Y-Axis
  HMC5983_Y_byte1 = HMC5983_Y_byte1 << 8;
  HMC5983_Y_value = HMC5983_Y_byte0 + HMC5983_Y_byte1;
  Y_milliGauss = HMC5983_Y_value * 0.00092;
  
  //---- Z-Axis
  HMC5983_Z_byte1 = HMC5983_Z_byte1 << 8;
  HMC5983_Z_value = HMC5983_Z_byte0 + HMC5983_Z_byte1;
  Z_milliGauss = HMC5983_Z_value * 0.00092;
  // ==============================
 
  // Correcting the heading with the geo_magnetic_declination_deg angle depending on your location
  // You can find your geo_magnetic_declination_deg angle at: http://www.ngdc.noaa.gov/geomag-web/
  // At zzz location it's 4.2 degrees => 0.073 rad
  // Haarlem                    2017-10-20  1° 4' E  ± 0° 22'  changing by  0° 9' E per year
  // Amsterdam                  2017-10-20  1° 9' E  ± 0° 22'  changing by  0° 9' E per year
  // Alkmaar 52.6324 4.7534     2017-10-20  1.09° E  ± 0.38°  changing by  0.14° E per year
  geo_magnetic_declination_deg = 1.09; // for our location
  
  //Calculating Heading
  headingDegrees = atan2(Y_milliGauss, X_milliGauss)* 180/PI + geo_magnetic_declination_deg;  // heading in rad. 
  
  // Correcting when signs are reveresed or due to the addition of the geo_magnetic_declination_deg angle
  if(headingDegrees <0) headingDegrees += 2*180;
  if(headingDegrees > 2*180) headingDegrees -= 2*180;
  
  //Sending the heading value through the Serial Port 
  #ifdef SHOW_DEBUGLEVEL2
  Serial.print("  headingDegrees=");
  Serial.print(headingDegrees);
  #endif
  return headingDegrees;
}

//----------------------------------------

void HMC5983_measure () {
  
  long compass = readCompass(); // 0..360 deg
  
// uint8_t compass_bin = ((compass/3) & 127) | (myBtn << 7) ;  // rescale 0-360 deg into 0 - 120 values and make sure it is not bigger than one byte
//  myLoraWanData[21] = compass_bin;
//  #ifdef SHOW_DEBUG
//  Serial.print(F("  compass=")); Serial.print(compass); Serial.print(F("  deg. bin=")); Serial.println(compass_bin);
//  #endif
}

void HMC5983_init () {  
  Compass_init();
  
  // as part of init, do one measurement to load values into send buffer
  HMC5983_measure();
}
