/*******************************************************************************
 * Modified By DenniZr & Marco van Schagen for Junior IOT - Smart City Challenge 2018 #juniorIOTchallenge2018
 * Modified By Marco van Schagen for Junior IOT Challenge 2018
 *******************************************************************************/ 
 
// build options for Arduino IDE: 
//   Lora32u4            - compile as adafruit feather 32u4

// Libraries to add to Arduino IDE from our libraries subfolder C:\arduino_port\GitHub\Junior-IOT-in-a-box-Project\libraries\ 
//    <NeoSWSerial.h>  --> \NeoSWSerial-master-DamiaBranch.zip
//    <NMEAGPS.h>      --> \NeoGPS-master.zip  --> and adjust them according to lines 45- below
//    <lmic_slim.h>    --> \lmic_slim.zip

// libraries to add from generic libraries
//   Seeed_BME280.h
 


//-------------- Message Byte Positions  ------------//
#define GPS_sendbufferStartByte 0 //9 bytes
#define internals_sendbufferStartByte 9 //17 bytes
#define BME280_sendbufferStartByte 26 //6 bytes
#define SDS_sendbufferStartByte 32 //4 bytes
#define MHZ19_sendbufferStartByte 36 //2 bytes
#define LeoEQ7_sendbufferStartByte 38 //16 bytes

#define PAYLOADSIZE 40 //Max 40    The size of the package to be sent
uint8_t  myLoraWanData[60];  // including byte[0]
//bool has_sent_allready = false; 
unsigned long datetime_gps=0;

//-------------- generic and Arduino Lora32u4 internals ------------//
//#define SHOW_DEBUG           // if SHOW_DEBUG is defined, some code is added to display some basic SHOW_DEBUG info
//#define SHOW_DEBUGLEVEL2     // if SHOW_DEBUGLEVEL2 is defined, some code is added to display deeper SHOW_DEBUG info
#include "juniorIOT_32u4internals.h"

//-------------- LoraWan libraries, mappings and things ------------//
#define LORAWAN_TX_INTERVAL_MIN 15  // seconds between LoraWan messages if change in values is detected
#define LORAWAN_TX_INTERVAL_MAX 240   // seconds between LoraWan messages
#define TXTRIGGER_gps_movement 8000 // will send if Min interval reached and GPS movement > value
    // gps coordinates in Long value means 52.6324510 translates into 526324510
    // 5000 trigger value means 25-30 meters


#include "keys.h"          // the personal keys to identify our own nodes, in a file outside GITHUB
#include "juniorIOT_LoraWan.h" // this is where all the LoraWan code and libraries are defined
// #include "junorIOT_RFM95_radio.h"  --> this allows device to device radio, no longer in use

//-------------- GPS things ------------//
#define GPS_FIX_HDOP   // to prevent eror: 'const class gps_fix' has no member named 'hdop'
#define GPS_RXD_PIN 10  // TXD on gps to rx arduino
#define GPS_TXD_PIN 11    // RXD on gps to tx arduino
#include "juniorIOT_GPS.h"  // this is where all GPS code and libraries are defined

//-------------- Compass HMC5983, BME280, other items  ------------//
#include <Wire.h> //I2C Arduino Library
  #include "juniorIOT_BME280.h" 
#include "juniorIOT_HMC5983.h" 
#include "juniorIOT_SDS011_021.h" 


void setup() {  
  pinMode(LEDPIN, OUTPUT);
  
  delay(2500);     // Give time to the ATMega32u4 port to wake up and be recognized by the OS.
  Serial.begin(115200);   // whether 9600 or 115200; the gps feed shows repeated char and cannot be interpreted, setting high value to release system time
    
  Serial.println(F("\nStarting device.")); 
  Serial.print(F("Setup() started. t=")); Serial.println(millis());

  // initialize all sensors and things
  GPS_init();
  internals_init();
  HMC5983_init();
    BME280_init();   Serial.print(F("  Completed: bme280 init. t=")); Serial.println(millis());  // needs debugging, locks if no BME280 is connected
  setup_pm();
  // once all values have been initialized, init Lora and send message to TTN
  LoraWan_init();  // --> init and also send one message 
    
  Serial.print(F("Setup() completed. t=")); Serial.println(millis());
}


void loop() {
  Serial.print(F("\n==== Loop starts. t=")); Serial.println(millis());
  digitalWrite(LEDPIN, !digitalRead(LEDPIN)); 

  // wait till we want to re-check the measurements
  while((millis() - last_check_time) < (LORAWAN_TX_INTERVAL_MIN * 1000L)) {
    delay(5000);   
    Serial.print(F("."));
  }
  Serial.println();
    
  // re-check all measurements
  GPS_measure();  // also gets current datetime
  Serial.print(F("\nRedo all measurements. t=")); Serial.println(millis());
  internals_measure();
  HMC5983_measure();
    BME280_measure();   Serial.print(F("  Completed: bme280 init. t=")); Serial.println(millis());  // needs debugging, locks if no BME280 is connected
  pm_measure();
  

  ////////// Now CHECK IF we need to send a LORAWAN update to the world  ///////////
  // switch the LMIC antenna to LoraWan mode
  if ( (millis() - last_lora_time) > ((LORAWAN_TX_INTERVAL_MAX) * 1000L)
      || (l_lat_movement > TXTRIGGER_gps_movement) 
      || (l_lon_movement > TXTRIGGER_gps_movement) ) {
    Serial.print(F("   - - - - About to send one LoraWan. ")); 
    last_lora_time = millis();
    lmic_slim_init();
    LoraWan_send();    
  } else {
    Serial.print(F("   - Not sending. ")); 
  }
  last_check_time = millis();
  
  Serial.print(F(" t=")); Serial.println(millis());
  Serial.print(F("     l_lat=")); Serial.print(l_lat);
  Serial.print(F("; l_lat_hist=")); Serial.print(l_lat_hist);
  Serial.print(F("; l_lat_movement=")); Serial.println(l_lat_movement);
  Serial.print(F("     l_lon=")); Serial.print(l_lon);
  Serial.print(F("; l_lon_hist=")); Serial.print(l_lon_hist);      
  Serial.print(F("; l_lon_movement=")); Serial.println(l_lon_movement);
      
  /////////// Loop again  //////////////
  #ifdef SHOW_DEBUGLEVEL2
  Serial.println(F("  End of loop. t=")); Serial.println(millis());
  #endif
}



