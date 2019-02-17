/*******************************************************************************
 * Modified By DenniZr & Marco van Schagen for Junior IOT - Smart City Challenge 2018 #juniorIOTchallenge2018
 * Modified By Marco van Schagen for Junior IOT Challenge 2018
 *******************************************************************************/ 
 
// build options: 
//   Lora32u4            - compile as adafruit feather 32u4
//   SS micro atmega32u4 - compile as ??   bricked --> use reset to GND at programming time 

// Libraries to add from our libraries subfolder: 
//    <NeoSWSerial.h>  --> C:\arduino_port\GitHub\Junior_Internet_of_Things_2018\libraries\NeoSWSerial-master-DamiaBranch.zip
//    <NMEAGPS.h>      --> C:\arduino_port\GitHub\Junior_Internet_of_Things_2018\libraries\NeoGPS-master.zip
//    <lmic_slim.h>    --> C:\arduino_port\GitHub\Junior_Internet_of_Things_2018\libraries\lmic_slim.zip
//    <RH_RF95.h>      --> C:\arduino_port\GitHub\Junior_Internet_of_Things_2018\libraries\RadioHead-1.79.zip
 
//#define DEBUG           // if DEBUG is defined, some code is added to display some basic debug info
//#define DEBUGLEVEL2     // if DEBUGLEVEL2 is defined, some code is added to display deeper debug info
//#define DEBUGRADIO      // if DEBUGLEVEL2 is defined, some code is added to display radio debug info
//#define doradio

#define VBATPIN A9
#define LEDPIN 13 
   // seconds between LoraWan messages
   // 240 = every 4 minutes,  was original 10/26
   //  60 = once a minute
   // MIN = shortest wait time if movement was detected
#define LORAWAN_TX_INTERVAL_MIN 15  // seconds between LoraWan messages
#define LORAWAN_TX_INTERVAL_MAX 240
#define TXTRIGGER_gps_movement 8000 // will send if Min interval reached and GPS movement > value
    // gps coordinates in Long value means 52.6324510 translates into 526324510
    // 5000 trigger value means 25-30 meters
   

//////////////////////////////////////////////
// LoraWan libraries, mappings and things
//////////////////////////////////////////////

//#include <SPI.h>  //MISO MOSI SCK stuff that was part of 2017 thing with rfm95
#define PAYLOADSIZE 38 // The size of the package to be sent
#include <avr/pgmspace.h>
#include <lmic_slim.h>     // the really cool micro-library, to replace our 2017 LMIC which filled 99% memory
#include "keys.h"          // the personal keys to identify our own nodes, in a file outside GITHUB

int TX_COMPLETE_was_triggered = 0;  // 20170220 added to allow full controll in main Loop
uint8_t  myLoraWanData[40];  // including byte[0]
unsigned long last_lora_time = millis(); // last time lorawan 
unsigned long last_check_time = millis(); // last time we checked for movement detection

//--------------Table of contents------------//
//////////////////////////////////////////////////////////
//// Kaasfabriek routines for gps
////////////////////////////////////////////
void doGPS();
void put_gpsvalues_into_sendbuffer();
void gps_init();
//////////////////////////////////////////////////
// Kaasfabriek routines for LMIC_slim for LoraWan
///////////////////////////////////////////////
void LoraWan_init();
void doOneLoraWan();
//////////////////////////////////////////////////
// Kaasfabriek routines for RFM95 radio to radio 
///////////////////////////////////////////////
void formatRadioPackage(uint8_t *loopbackToData);
void doOneRadio();
void halt_stressed();
void Radio_init();
void decodeReply(uint8_t buf[], bool debugToSerial);
///////////////////////////////////////////////
//  some other measurements
///////////////////////////////////////////
double GetTemp(void);
long readVbat();
void put_TimeToFix_into_sendbuffer(int TimeToFix_Seconds);
//////////////////////////////////////////////////////////
//// Compass HMC5983
////////////////////////////////////////////
void Compass_init();
long readCompass();
void put_Compass_and_Btn_into_sendbuffer();
///////////////////////////////////////////////
//  arduino init and main
///////////////////////////////////////////
void setup();
void loop();
//--------------/Table of contents------------//



//////////////////////////////////////////////////
// Kaasfabriek routines for LMIC_slim for LoraWan
///////////////////////////////////////////////

void lmic_slim_init() {
  #ifdef DEBUG
  Serial.print(F("Start: lmic_slim_init. milis=")); Serial.println(millis());  
  #endif
  
  spi_start();
  pinMode(SS_pin, OUTPUT);                                                                  
  Serial.println(SS_pin);
  pinMode(SCK_pin, OUTPUT);                                         
  Serial.println(SCK_pin);
  pinMode(MOSI_pin, OUTPUT);
  Serial.println(MOSI_pin);
  digitalWrite(SCK_pin, LOW);            // SCK low
  digitalWrite(SS_pin, HIGH);            // NSS high
  delay(10);
  //writeReg(0x01, 0x08);
  delay(10);
  //radio_init();  // that is in the LMIC_slim library
  delay(10);
  uint8_t appskey[sizeof(APPSKEY)];
  uint8_t nwkskey[sizeof(NWKSKEY)];
  memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
  memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));

  Serial.print("  TTN registration old style:  LMIC_setSession (DEVADDR, nwkskey, appskey) \n");
  Serial.print("    DEVADDR = ");Serial.print(DEVADDR, HEX);Serial.print(" \n");
  Serial.print("    nwkskey = ");
    //Serial.print(nwkskey);
    for(int i=0;i<16;i++){if(nwkskey[i]<16)Serial.print('0');Serial.print(nwkskey[i],HEX);Serial.print(" ");} Serial.print(" \n");
  Serial.print("    appskey = ");
    //Serial.print(appskey);
    for(int i=0;i<16;i++){if(appskey[i]<16)Serial.print('0');Serial.print(appskey[i],HEX);Serial.print(" ");} Serial.print(" \n");
  Serial.println();
  
  LMIC_setSession (DEVADDR, nwkskey, appskey);
  
  LMIC_LORARegModemConfig (0x72, 0b01110100, 0x04);  // LORARegModemConfig1, LORARegModemConfig2, LORARegModemConfig3
    // USE THESE SETS FOR CORRECT LORA SPEC:
    //        0x72, 0b11000100, 0x0C  // BW=125, Coding Rate=4/5, SF12, mobile
    //        0x72, 0b10110100, 0x0C  // BW=125, Coding Rate=4/5, SF11, mobile
    //        0x72, 0b10100100, 0x04  // BW=125, Coding Rate=4/5, SF10, static
    //        0x72, 0b10010100, 0x04  // BW=125, Coding Rate=4/5, SF9, static 
    //        0x72, 0b10000100, 0x04  // BW=125, Coding Rate=4/5, SF8, static
    //        0x72, 0b01110100, 0x04  // BW=125, Coding Rate=4/5, SF7, static -> this is preferred setting for generic nodes and is the TTNmapper default
    // LORARegModemConfig1
    //       0x72 is normal BW=125 en Coding Rate=4/5, all other messages I see on our TTN basestation have BW=125 en Coding Rate=4/5
    // LORARegModemConfig2 
    //       nnnn----   Spreading Factor (bit 7..4)
    //       ----0---   TxContinuousMode =0 normal mode (bit 3)  
    //       -----1--   RxPayloadCrcOn = 1 CRC ON (bit 2)  
    //       ------00   SymbTimeout(9:8)=00 default (bit 1..0)
    // LORARegModemConfig3, bit 3 van RegModemConfig3, spec van de RFM95 zegt: bit 3: MobileNode, type: rw, Default: 0x00, Value: 0 : Use for static node
    //       0x0C  originally by Rene Harte, IOT-partners for SF11, SF12  
    //       0x04  for SF7.....SF10 
    // Airtime voor 5 bytes payload = 13 x 2^(SF-6) ms. 
    //       Number of sec per message with 30-50 bytes: roughly SF12 = 2 seconds, SF10 = 0,5 sec, SF8 = 120 msec, SF7= 60 msec. One device is allowed a total of 30 seconds per day airtime.
    //       Message frequency allowed SF12 = 15 messages per day = 1 message every 90 minutes; SF11 = 1 per 45 min; SF10 = 1 per 24 min, SF9 = 1 per 12 min, SF8 = 1 per 6 min, SF7 = 1 per 3 min roughly
    // For your received messages:
    //       RSSI  Received signal strength indicator, as low as -120 is told to be workable, but I have seen it work even lower. I get -120 from my work table. Does not change with SF.
    //       SNR   Signal to noise ratio, as low as -20dB seems workable. I get -5 from my work table.
}

void print_myLoraWanData() {
  #ifdef DEBUGLEVEL2
  Serial.print(F("  myLoraWanData = [")); 
  //Serial.print((char*)myLoraWanData); Serial.println("]"); Serial.print(F("                  [ "));  
  for(int i=0; i<PAYLOADSIZE; i++) {  
    if (myLoraWanData[i] < 16) Serial.print(F("0")); 
    Serial.print(myLoraWanData[i], HEX); 
    Serial.print(F(" "));  
  }  
  Serial.println();
  #endif 
}

void doOneLoraWan() {
  #ifdef DEBUG
  Serial.print(F("Start: - - - - - - - - - - -- - - - doOneLoraWan. milis=")); Serial.println(millis());
  print_myLoraWanData();
  #endif
  
  LMIC_setTxData2(myLoraWanData, PAYLOADSIZE);
  radio_init();                                                       
  delay (10);
  //digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(LEDPIN, !digitalRead(LEDPIN));
  
  //#ifdef DEBUGLEVEL2
  Serial.print(F("  txLora. milis=")); Serial.println(millis());
  //#endif
  
  txlora();
  
  //#ifdef DEBUGLEVEL2
  Serial.print(F("  txLora completed. milis=")); Serial.println(millis());
  //#endif
  
  delay(200);           // this is a simple wait with no checking for TX Ready. Sdjust this for your SF.
                          // Airtime voor 5 bytes payload = 13 x 2^(SF-6) ms. 
                          // with 30-50 bytes: SF12 = 2 seconds, SF10 = 0,5 sec, SF8 = 120 msec, SF7= 70 msec. One device has 30 seconds per day airtime.
  //digitalWrite(LED_BUILTIN, LOW);
  
  #ifdef DEBUGLEVEL2
  Serial.print(F("  send time delay completed. milis=")); Serial.println(millis());
  #endif
  
  digitalWrite(LEDPIN, !digitalRead(LEDPIN));
  setopmode(0x00);                     // opmode SLEEP; better not tell lorawan to go to sleep before message is done
}

void setup() {
  pinMode(LEDPIN, OUTPUT);
  
  delay(2500);     // Give time to the ATMega32u4 port to wake up and be recognized by the OS.
  Serial.begin(115200);   // whether 9600 or 115200; the gps feed shows repeated char and cannot be interpreted, setting high value to release system time
  lmic_slim_init();
  doOneLoraWan(); 
}

void loop() {}



