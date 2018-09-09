/*******************************************************************************
 * Modified By DenniZr & Marco van Schagen for Junior IOT - Smart City Challenge 2018 #juniorIOTchallenge2018
 * Modified By Marco van Schagen for Junior IOT Challenge 2018
 *******************************************************************************/ 
 
// build options: 
//   Lora32u4            - compile as adafruit feather 32u4
//   SS micro atmega32u4 - compile as ??   bricked --> use reset to GND at programming time 

// Libraries to add from our libraries subfolder C:\arduino_port\GitHub\Junior-IOT-in-a-box-Project\libraries\ 
//    <NeoSWSerial.h>  --> \NeoSWSerial-master-DamiaBranch.zip
//    <NMEAGPS.h>      --> \NeoGPS-master.zip  --> and adjust them according to lines 45- below
//    <lmic_slim.h>    --> \lmic_slim.zip
//   // <RH_RF95.h>      --> \RadioHead-1.79.zip

// libraries to add 
//   Seeed_BME280.h
 
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
// GPS libraries, mappings and things
//////////////////////////////////////////////
#define GPS_FIX_HDOP   // to prevent eror: 'const class gps_fix' has no member named 'hdop'
#define GPS_RXD_PIN 10  // TXD on gps to rx arduino
#define GPS_TXD_PIN 11    // RXD on gps to tx arduino

#define btnPin  A0 
#define temtPin  A3
#define temt2Pin  A4 

#include <NeoSWSerial.h>  //  We now use NeoSWSerial for lower footprint end better performance than SoftwareSerial
  // an issue with Leonardo-types is fixed in branch, yet to be merged into main version library. So you may need to remove all your NeoSWSerial libraries and add \libraries\NeoSWSerial-master-DamiaBranch.zip
NeoSWSerial ss(GPS_RXD_PIN, GPS_TXD_PIN);

// You must enable/change these lines in NMEAGPS_cfg.h:
//          #define NMEAGPS_PARSE_GLL  // juniorIOTchallenge2018
//          #define LAST_SENTENCE_IN_INTERVAL NMEAGPS::NMEA_GLL  // juniorIOTchallenge2018
// and in GPSfix_cfg.h:         
//          #define GPS_FIX_HDOP  // juniorIOTchallenge2018
#include <NMEAGPS.h>       // We now use NmeaGps (or NeoGps) as it understands newer GNSS (default reads only GGA, RMC)
static NMEAGPS gps;    // This parses the GPS characters

long gps_fix_count = 0;
long gps_nofix_count = 0;
unsigned long gps_last_time = millis();
//unsigned long gps_gets_time = 5000;



//////////////////////////////////////////////
// LoraWan libraries, mappings and things
//////////////////////////////////////////////

//#include <SPI.h>  //MISO MOSI SCK stuff that was part of 2017 thing with rfm95
#define PAYLOADSIZE 38 // The size of the package to be sent
#include <avr/pgmspace.h>  // not sure why this one
#include <lmic_slim.h>     // the really cool micro-library, to replace our 2017 LMIC which filled 99% memory
#include "keys.h"          // the personal keys to identify our own nodes, in a file outside GITHUB

int TX_COMPLETE_was_triggered = 0;  // 20170220 added to allow full controll in main Loop
uint8_t  myLoraWanData[40];  // including byte[0]
unsigned long last_lora_time = millis(); // last time lorawan 
unsigned long last_check_time = millis(); // last time we checked for movement detection

//////////////////////////////////////////////////////////
//// Compass HMC5983, BME280, other items
////////////////////////////////////////////
#include <Wire.h> //I2C Arduino Library
#include "Seeed_BME280.h"

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


//--------------Table of contents------------//
//// Kaasfabriek routines for gps
void doGPS();
void put_gpsvalues_into_sendbuffer();
void gps_init();

// Kaasfabriek routines for LMIC_slim for LoraWan
void LoraWan_init();
void doOneLoraWan();

// Kaasfabriek routines for RFM95 radio to radio 
void formatRadioPackage(uint8_t *loopbackToData);
void doOneRadio();
void halt_stressed();
void Radio_init();
void decodeReply(uint8_t buf[], bool debugToSerial);

//  some other measurements
double GetTemp(void);
double GetTemp_Atmega32(void);
double GetTemp_Atmega32u4(void);
long readVbat();
void put_TimeToFix_into_sendbuffer(int TimeToFix_Seconds);

//// Compass HMC5983, BME280 and other I2C things
void Compass_init();
long readCompass();
void put_Compass_and_Btn_into_sendbuffer();
void BME280_init();
void readBME();
void put_BME_into_sendbuffer();

//  arduino init and main
void setup();
void loop();
//--------------/Table of contents------------//


//////////////////////////////////////////////////////////
//// Kaasfabriek routines for gps
////////////////////////////////////////////
long l_lat, l_lon, l_alt, l_lat_hist, l_lon_hist, l_lat_movement, l_lon_movement;
int hdopNumber; 



void put_gpsvalues_into_sendbuffer() {
  #ifdef DEBUG
  Serial.println(F("Start: put_gpsvalues_into_sendbuffer"));
  
  #endif
  //   GPS reading = Satellite time hh:mm:18, lat 526326595, lon 47384133, alt 21, hdop 990, sat count = 12
  // With the larger precision in LONG values in NMEAGPS, 
  //    when rescaling, the values exceed the range for type LONG  -2.147.483.648 .. 2.147.483.647
  //    so we need to use DOUBLE with 15 digits precision, not preferred is FLOAT with 7 digits
  //    our values such as 526326595 have 9 digits
    
  const double shift_lat     =    90. * 10000000.;                 // range shift from -90..90 into 0..180, note: 
                                                                 //      NMEAGPS long lat&lon are degree values * 10.000.000
                                                                 //      TynyGPS long lat&lon are degree values * 1.000.000
  const double max_old_lat   =   180. * 10000000.;                 // max value for lat is now 180
  const double max_3byte     =         16777215.;                   // max value that fits in 3 bytes
  double lat_DOUBLE         = l_lat;                              // put 4byte LONG into a more precise floating point to prevent rounding during calcs 
  lat_DOUBLE = (lat_DOUBLE + shift_lat) * max_3byte / max_old_lat; // rescale into 3 byte integer range
  uint32_t LatitudeBinary  = lat_DOUBLE;                          // clips off anything after the decimal point    
  const double shift_lon     =   180. * 10000000.;                 // range shift from -180..180 into 0..360
  const double max_old_lon   = 360. * 10000000.;                   // max value longitude is now 360, note the value is too big for Long type
  double lon_DOUBLE = l_lon;                                      // put the 4byte LONG into a precise floating point memory space
  lon_DOUBLE = (lon_DOUBLE + shift_lon) * max_3byte / max_old_lon; // rescale into 3 byte integer range
  uint32_t LongitudeBinary = lon_DOUBLE;                          // clips off anything after the decimal point  
  uint16_t altitudeBinary  = l_alt    ;                          // we want altitudeGps in meters, note:
                                                                 //      NMEAGPS alt.whole is meter value ---> checked in Poznan, no need to divide by 10
                                                                 //      TynyGPS long alt is meter value * 100
  if (l_alt<0) altitudeBinary=0;                                 // unsigned int wil not allow negative values and warps them to huge number  
  uint8_t HdopBinary = hdopNumber/100;                           // we want horizontal dillution, good is 2..5, poor is >20. Note:
                                                                 //      NMEAGPS outputs an indoor value of 600..1000. Let's divide by 100
                                                                 //      from TinyGPS horizontal dilution of precision in 100ths? We succesfully divided by 10
                                                                 //      TinyGPSplus seems the same in 100ths as per MNEMA string. We succesfully divided by 10
  
//  Serial.print(F("  shift_lat = "));Serial.println(shift_lat);
//  Serial.print(F("  max_old_lat = "));Serial.println(max_old_lat);
//  Serial.print(F("  max_3byte = "));Serial.println(max_3byte);
//  Serial.print(F("  l_lat = "));Serial.println(l_lat);
//  Serial.print(F("  lat_float = "));Serial.println(lat_float);
//  Serial.print(F("  LatitudeBinary = "));Serial.println(LatitudeBinary);
//  
//  Serial.print(F("\n  shift_lon = "));Serial.println(shift_lon);
//  Serial.print(F("  max_old_lon = "));Serial.println(max_old_lon);
//  Serial.print(F("  l_lon = "));Serial.println(l_lon);
//  Serial.print(F("  lon_float = "));Serial.println(lon_float);
//  Serial.print(F("  LongitudeBinary = "));Serial.println(LongitudeBinary);
//  
//  Serial.print(F("\n  l_alt = "));Serial.println(l_alt);
//  Serial.print(F("  altitudeBinary = "));Serial.println(altitudeBinary);
//  
//  Serial.print(F("\n  hdopNumber = "));Serial.println(hdopNumber);
//  Serial.print(F("  HdopBinary = "));Serial.println(HdopBinary);
  
  myLoraWanData[0] = LatitudeBinary >> 16;
  myLoraWanData[1] = LatitudeBinary >> 8;
  myLoraWanData[2] = LatitudeBinary;
  myLoraWanData[3] = LongitudeBinary >> 16;
  myLoraWanData[4] = LongitudeBinary >> 8;
  myLoraWanData[5] = LongitudeBinary;
  // altitudeGps in meters into unsigned int
  myLoraWanData[6] = altitudeBinary >> 8;
  myLoraWanData[7] = altitudeBinary;
  // hdop in tenths of meter
  myLoraWanData[8] = HdopBinary;

  print_myLoraWanData();
  
//  Dummy satellite values: time hh:mm:5, lat 52632400, lon 4738800, alt 678, hdop 2345, sat count = 12
//       0  1  2  3  4  5  6  7  8   
//     [87 7C 49 80 56 44 02 A6 17 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  .. ]

}

bool process_gps_datastream(const gps_fix & fix) {   // constant pointer to fix object
  //unsigned long age; 
  //bool GPS_values_are_valid = true;
    
  //if (fix.valid.location && fix.dateTime.seconds > 0) {   
  if (fix.valid.location ) {    
    l_lat = fix.latitudeL();
    l_lon = fix.longitudeL();
    l_alt = fix.alt.whole;
    //l_alt = fix.altitude_cm;
    hdopNumber = fix.hdop;
    
    Serial.print( fix.dateTime.year ); Serial.print(F(" "));  
    Serial.print( fix.dateTime.month ); Serial.print(F(" "));  
    Serial.print( fix.dateTime.date ); Serial.print(F(" "));  
    Serial.print( fix.dateTime.hours ); Serial.print(F(":"));  
    Serial.print( fix.dateTime.minutes ); Serial.print(F(":")); 
    Serial.print( fix.dateTime.seconds ); Serial.print(F(" "));  
    Serial.print( fix.dateTime ); Serial.println(F(" GPS datetime, "));  
    
    // serial print commands will take time and may affect the gps read
    #ifdef DEBUGLEVEL2
    Serial.print(F("  Satellite time hh:mm:")); Serial.print( fix.dateTime.seconds ); Serial.print(F(", "));
    Serial.print(F("lat ")); Serial.print( l_lat  ); Serial.print(F(", "));
    Serial.print(F("lon ")); Serial.print( l_lon ); Serial.print(F(", "));
    Serial.print(F("alt ")); Serial.print( l_alt ); Serial.print(F(", "));
    Serial.print(F("hdop ")); Serial.print( hdopNumber ); Serial.print(F(", "));
    Serial.print(F("sat = ")); Serial.print( fix.satellites );
    Serial.print( fix.dateTime ); Serial.print(" datetime, ");    
    //Serial.print( fix.latitude(), 6 ); // floating-point display
    //Serial.print( fix.longitude(), 6 ); // floating-point display
    //   Satellite time hh:mm:18, lat 526326595, lon 47384133, alt 21, hdop 990, sat count = 12
    
    Serial.println();
    #endif
    return true;
  } else {
    //#ifdef DEBUG
    Serial.print( "(no valid location) " );
    //#endif
    return false;
  }  
}

void doGPS(unsigned long gps_listen_timeout) {
  //#ifdef DEBUG
  Serial.print(F("Start: doGPS. timeout=")); Serial.println(gps_listen_timeout);Serial.print(F(" t=")); Serial.println(millis());
  //#endif
  
  bool hasFix = false;
  unsigned long gps_listen_startTime = millis(); 

  
  //now listen to gps till fix or time-out, once gps has a fix, the refresh should be ready within 2 data reads = less than 3 sec
  // gps read command:
  //#ifdef DEBUGLEVEL2
  Serial.println(F("Listen to GPS stream:"));
  //#endif
  while((millis() - gps_listen_startTime) < (gps_listen_timeout * 1000L) && !hasFix) {
    // for NMEAgps
    while (gps.available(ss) && !hasFix) {
      hasFix = process_gps_datastream(gps.read()); 
    }
  }    
  //#ifdef DEBUGLEVEL2
  Serial.println(F("Completed listening to GPS."));
  //#endif
  if(hasFix) {
    // movement detection
    if(l_lat_hist==0) l_lat_hist=l_lat;
    if(l_lon_hist==0) l_lon_hist=l_lon;
    l_lat_movement = abs(l_lat_hist - l_lat);
    l_lon_movement = abs(l_lon_hist - l_lon);
    l_lat_hist= (2*l_lat_hist + l_lat)/3; 
    l_lon_hist= (2*l_lon_hist + l_lon)/3; 
  }
}
void doGPS_and_put_values_into_sendbuffer() {
  #ifdef DEBUG
  Serial.print(F("Start: doGPS_and_put_values_into_sendbuffer. t=")); Serial.println(millis());
  #endif
  
  doGPS(3);
  // put gps values into send buffer
  put_gpsvalues_into_sendbuffer();
  #ifdef DEBUGLEVEL2
  Serial.print(F("\Completed: doGPS_and_put_values_into_sendbuffer. t=")); Serial.println(millis());
  #endif
}

void gps_init() {  
  //#ifdef DEBUG
  Serial.print(F("GPS init. t=")); Serial.println(millis());
  //#endif
    
  // load the send buffer with dummy location 0,0. This location 0,0 is recognized as dummy by TTN Mapper and will be ignored
  //l_lat = 526324000; l_lon = 47388000; l_alt = 678; hdopNumber = 23459;   // Alkmaar
  l_lat = 0; l_lon = 0; l_alt = 678; hdopNumber = 99999;   // the zero position
  l_lat_hist = 0; l_lon_hist = 0; l_lat_movement = 0; l_lon_movement = 0; // movement detection
  put_gpsvalues_into_sendbuffer(); 
  
//  Serial.print( F("The NeoGps people are proud to show their smallest possible size:\n") );
//  Serial.print( F("NeoGps, fix object size = ") ); Serial.println( sizeof(gps.fix()) );
//  Serial.print( F("NeoGps, NMEAGPS object size = ") ); Serial.println( sizeof(gps) );

  //Serial.flush();
  ss.begin(9600);
  
  //gps_requestColdStart();  // DO NOT USE: it seems this does a FACTORY RESET and delays getting a solid fix
//  gps_SetMode_gpsRfOn();
  gps_setStrings();
//  gps_setNavMode(7); // 2=stationary, 3=pedestrian, 4=auto, 5=Sea, 6=airborne 1g, 7=air 2g, 8=air 4g
  
//  gps_setPowerMode(1);  // 1=max power, 2=eco, 3=cyclic power save

  //assume fix was found, go to airborne
  // Lessons learned,
  //       my gps does not find fix while on the ground in mode 6
  //       does find fix in mode 7, but no re-fix after RF-off and RF-on
  //       so for now we keep in mode 4 OR experiment with powermodes instead of RF_off
  // gps_setNavMode(7); // 2=stationary, 3=pedestrian, 4=auto, 5=Sea, 6=airborne 1g, 7=air 2g, 8=air 4g -- with 6 no 2d fix supported
//   gps_read_until_fix_or_timeout(30 * 60);  // after factory reset, time to first fix can be 15 minutes (or multiple).  gps needs to acquire full data which is sent out once every 15 minutes; sat data sent out once every 5 minutes
//gps_read_chars(200);
  //gps_setPowerMode(2);
}

void gps_setStrings() {
  #ifdef DEBUG
  serial.print(F("Start: gps_setStrings. t=")); Serial.println(millis());
  #endif

  // Turning ON or OFF  GPS NMEA strings 
  // we need lat, lon, alt, HDOP, date  --> keep GGA, RMC

  // GLL = Lat/Lon time fix
  //ss.print(F("$PUBX,40,GLL,0,0,0,0*5C\r\n"));  // GLL OFF
  ss.print(F("$PUBX,40,GLL,1,1,1,0*5D\r\n"));  // GLL ON

  // ZDA = date, time
  ss.print(F("$PUBX,40,ZDA,0,0,0,0*44\r\n"));  // ZDA OFF
  //ss.print(F("$PUBX,40,ZDA,1,1,1,0*45\r\n"));  // ZDA ON 
  
  // VTG = Vector Track and speed over ground
  ss.print(F("$PUBX,40,VTG,0,0,0,0*5E\r\n"));  // VTG OFF
  //ss.print(F("$PUBX,40,VTG,1,1,1,0*5F\r\n"));  // VTG ON

  // GSV = Satellite in View. #sentences,sentence#,#sat,[sat PRN#, elev degr, azi degr, SNR,] *check
  ss.print(F("$PUBX,40,GSV,0,0,0,0*59\r\n"));  //GSV OFF
  //ss.print(F("$PUBX,40,GSV,1,1,1,0*58\r\n"));  //GSV ON

  // RMC = recommended minimum, no Alt
  // ss.print(F("$PUBX,40,RMC,0,0,0,0*47\r\n"));    // RMC OFF
  ss.print(F("$PUBX,40,RMC,1,1,1,0*46\r\n"));      // RMC ON 

  // GSA = Overall Satelite status. Auto/Manual,1/2/3 D fix, PRN1, ...PRN12 satt id, pdop,hdop,vdop,*check
  ss.print(F("$PUBX,40,GSA,0,0,0,0*4E\r\n"));  // GSA OFF
  //ss.print(F("$PUBX,40,GSA,1,1,1,0*4F\r\n"));  // GSA ON

  // GGA = Fix information. time,lat,N,lon,E,fix qual,num sat,hor dilution, alt,M,height geoid,M,time since DGPS,DGPS id, *check
  // ss.println(F("$PUBX,40,GGA,0,0,0,0*5A"));   // GGA OFF
  ss.println(F("$PUBX,40,GGA,1,1,1,0*5B"));   // GGA ON

  //GRS
  //$PUBX,40,GRS,0,0,0,0*5D // Turn OFF
  //$PUBX,40,GRS,1,1,1,0*5C // Turn ON

  //GST
  //$PUBX,40,GRS,0,0,0,0*5D // Turn OFF
  //$PUBX,40,GRS,1,1,1,0*5C // Turn ON

  //gps_read_chars(300);
  
  unsigned long gps_listen_startTime = millis(); 
  unsigned long gps_timeout = 3;  // sec
  
  int times_without_char=0;
  while((millis() - gps_listen_startTime) < (gps_timeout * 1000L)) {
    // for debugging
    if (ss.available()) {
      char c = ss.read();
      Serial.write(c);
    }
    if(times_without_char++>30 && times_without_char<60) Serial.write(".");
  }
  Serial.println();
  
  // #define LAST_SENTENCE_IN_INTERVAL NMEAGPS::NMEA_RMC

  // our GN-801 works well
  //     used to output: $GNVTG, $GNGGA, $GNGSA, $GPGSV, $GLGSV, $GNGLL. 
  //     Changed to --> RMC, GGA, GLL
}

//////////////////////////////////////////////////
// Kaasfabriek routines for LMIC_slim for LoraWan
///////////////////////////////////////////////

void lmic_slim_init() {
  //#ifdef DEBUG
  Serial.print(F("lmic_slim_init. t=")); Serial.println(millis());  
  //#endif
  
  spi_start();
  pinMode(SS_pin, OUTPUT);                                                                  
  pinMode(SCK_pin, OUTPUT);                                         
  pinMode(MOSI_pin, OUTPUT);
  digitalWrite(SCK_pin, LOW);            // SCK low
  digitalWrite(SS_pin, HIGH);            // NSS high
  delay(10);
  writeReg(0x01, 0x08);
  delay(10);
  radio_init();  // that is in the LMIC_slim library
  delay(10);
  uint8_t appskey[sizeof(APPSKEY)];
  uint8_t nwkskey[sizeof(NWKSKEY)];
  memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
  memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
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
  Serial.print(F("Start: - - - - - - - - - - -- - - - doOneLoraWan. t=")); Serial.println(millis());
  print_myLoraWanData();
  #endif
  
  LMIC_setTxData2(myLoraWanData, PAYLOADSIZE);
  radio_init();                                                       
  delay (10);
  //digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(LEDPIN, !digitalRead(LEDPIN));
  
  #ifdef DEBUGLEVEL2
  Serial.print(F("  txLora. t=")); Serial.println(millis());
  #endif
  
  txlora();
  
  #ifdef DEBUGLEVEL2
  Serial.print(F("  txLora completed. t=")); Serial.println(millis());
  #endif
  
  delay(200);           // this is a simple wait with no checking for TX Ready. Sdjust this for your SF.
                          // Airtime voor 5 bytes payload = 13 x 2^(SF-6) ms. 
                          // with 30-50 bytes: SF12 = 2 seconds, SF10 = 0,5 sec, SF8 = 120 msec, SF7= 70 msec. One device has 30 seconds per day airtime.
  //digitalWrite(LED_BUILTIN, LOW);
  
  #ifdef DEBUGLEVEL2
  Serial.print(F("  send time delay completed. t=")); Serial.println(millis());
  #endif
  
  digitalWrite(LEDPIN, !digitalRead(LEDPIN));
  setopmode(0x00);                     // opmode SLEEP; better not tell lorawan to go to sleep before message is done

  //just after the send is completed, we push the location data into buffer for prev location - dark spot finder  
  myLoraWanData[9] = myLoraWanData[0];
  myLoraWanData[10] = myLoraWanData[1];
  myLoraWanData[11] = myLoraWanData[2];
  
  myLoraWanData[12] = myLoraWanData[3];
  myLoraWanData[13] = myLoraWanData[4];
  myLoraWanData[14] = myLoraWanData[5];
  
  myLoraWanData[15] = myLoraWanData[6];
  myLoraWanData[16] = myLoraWanData[7];
  myLoraWanData[17] = myLoraWanData[8];
  
  #ifdef DEBUGLEVEL2
  Serial.print(F("Completed: Do one lora. t=")); Serial.println(millis());
  #endif
}

///////////////////////////////////////////////
//  some other measurements
///////////////////////////////////////////

double GetTemp(void) { 
  //return GetTemp_Atmega32();
  return GetTemp_Atmega32u4();
}
double GetTemp_Atmega32(void) {
  // for atmega32    http://playground.arduino.cc/Main/InternalTemperatureSens
  // for atmega32u4  http://www.narkidae.com/research/atmega-core-temperature-sensor/
  unsigned int wADC;
  double t;
  
  // The internal temperature has to be used with the internal reference of 1.1V. ATmega32U4 has 2.56V ref instead of 1.1?
  // Set the internal reference and mux.
  ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3));
  ADCSRA |= _BV(ADEN);  // enable the ADC
  delay(20);            // wait for voltages to become stable.
  ADCSRA |= _BV(ADSC);  // Start the ADC

  // Detect end-of-conversion
  while (bit_is_set(ADCSRA,ADSC));  
  // Reading register "ADCW" takes care of how to read ADCL and ADCH.
  wADC = ADCW;
  
  t = (wADC - 324.31 ) / 1.22;  // The offset of 324.31 could be wrong. It is just an indication.
  // The returned temperature is in degrees Celsius.
  //t = t - 10; // before this correction we had readings of 30 in a room which was 20 deg C, readings of 20 outdoors when it was 6 deg C

  return (t);
}
double GetTemp_Atmega32u4(void) {
  // for atmega32    http://playground.arduino.cc/Main/InternalTemperatureSens
  // for atmega32u4  http://www.narkidae.com/research/atmega-core-temperature-sensor/

  
  //void setupADC_Atmega32u4(){
  //cli();  //Disable global interrupts

  //ADC Multiplexer Selection Register
  ADMUX = 0;
  ADMUX |= (1 << REFS1);  //Internal 2.56V Voltage Reference with external capacitor on AREF pin
  ADMUX |= (1 << REFS0);  //Internal 2.56V Voltage Reference with external capacitor on AREF pin
  ADMUX |= (0 << MUX4);  //Temperature Sensor - 100111
  ADMUX |= (0 << MUX3);  //Temperature Sensor - 100111
  ADMUX |= (1 << MUX2);  //Temperature Sensor - 100111
  ADMUX |= (1 << MUX1);  //Temperature Sensor - 100111
  ADMUX |= (1 << MUX0);  //Temperature Sensor - 100111

  //ADC Control and Status Register A 
  ADCSRA = 0;
  ADCSRA |= (1 << ADEN);  //Enable the ADC
  delay(20);            // wait for voltages to become stable.
  ADCSRA |= (1 << ADPS2);  //ADC Prescaler - 16 (16MHz -> 1MHz)

  //ADC Control and Status Register B 
  ADCSRB = 0;
  ADCSRB |= (1 << MUX5);  //Temperature Sensor - 100111

  //sei();  //Enable global interrupts
  //}
  
  //int getTemp(){
  ADCSRA |= (1 << ADSC);  //Start temperature conversion
  delay(20);            // wait for voltages to become stable.
  while (bit_is_set(ADCSRA, ADSC));  //Wait for conversion to finish
  byte low  = ADCL;
  byte high = ADCH;
  int TEMP_OFFSET = -7;
  int temperature = (high << 8) | low;  //Result is in kelvin
  return temperature - 273 + TEMP_OFFSET;
  //}
}

long readVccCPU() {  //http://dumbpcs.blogspot.nl/2013/07/arduino-secret-built-in-thermometer.html
  long result;
  // Read 1.1V reference against AVcc 
  //ATmega32U4 has 2.56V ref instead of 1.1?
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(50); // Wait for Vref to settle  // was 2
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result; // Back-calculate AVcc in mV
  return result;
}

long readVbat() {
  long result;
  float measuredvbat = analogRead(VBATPIN);
    // devide by 1024 to convert to voltage
    // we divided by 2 using 2x 100M Ohm, so multiply x2
    // Multiply by 3.3V, our reference voltage
    // *1000 to get milliVolt
  measuredvbat *= 6600. / 1024.;         
  result = measuredvbat;
//  Serial.print(F("  Vbat=")); 
//  Serial.print(result);
//  Serial.println(F(" milliVolt"));
  return result;
}

void put_Compass_and_Btn_into_sendbuffer() {
  long compass = readCompass(); // 0..360 deg
  
  // now add a bit for BTN 
  uint8_t myBtn = 0;  
  int btnValue = analogRead(btnPin);
  if (btnValue>100) myBtn=1;

  // light measurements  
  int temtValue = analogRead(temtPin);
  delay(1);
  int temt2Value = analogRead(temt2Pin);
  
  Serial.print(F("btnValue = "));
  Serial.print( btnValue);
  Serial.print(F("    temtValue = "));
  Serial.print( temtValue);
  Serial.print(F("    temt2Value = "));
  Serial.print( temt2Value);
  Serial.println("");

  uint8_t compass_bin = ((compass/3) & 127) | (myBtn << 7) ;  // rescale 0-360 deg into 0 - 120 values and make sure it is not bigger than one byte
   
  myLoraWanData[21] = compass_bin;
  #ifdef DEBUG
  Serial.print(F("  compass=")); Serial.print(compass); Serial.print(F("  deg. bin=")); Serial.println(compass_bin);
  #endif
}

void put_Volts_and_Temp_into_sendbuffer() {

  long vcc = readVccCPU();
  //long vcc = 0;
  uint8_t vcc_bin = vcc/20 ;  // rescale 0-5100 milli volt into 0 - 255 values and make sure it is not bigger than one byte
  myLoraWanData[18] = vcc_bin;
  #ifdef DEBUG
  Serial.print(F("  Vcc=")); Serial.print(vcc); Serial.print(F(" mV. bin=")); Serial.println(vcc_bin);
  #endif
  
  double cpuTemp = GetTemp();
  uint8_t cpuTemp_bin = cpuTemp + 100;   // rescale -100 to 155 into 0 - 255 values and make sure it is not bigger than one byte
  myLoraWanData[19] = cpuTemp_bin;
  //#ifdef DEBUG
  Serial.print(F("  CpuTemp=")); Serial.print(cpuTemp); Serial.print(F(" bin=")); Serial.println(cpuTemp_bin);
  //#endif
  
  long vbat = readVbat();
  uint8_t vbat_bin = vbat/20 ;  // rescale 0-5100 milli volt into 0 - 255 values and make sure it is not bigger than one byte
  myLoraWanData[20] = vbat_bin;
  //#ifdef DEBUG
  Serial.print(F("  Vbat=")); Serial.print(vbat); Serial.print(F(" mV. bin=")); Serial.println(vbat_bin);
  //#endif
}

void put_TimeToFix_into_sendbuffer(int TimeToFix_Seconds) {  // time to fix onto gps coordinates
//  int TimeToFix_Calculate;  // this helps to calculate but no round-off yet
//  if ( TimeToFix_Seconds < 0) {
//    TimeToFix_Calculate=0;
//  } else if ( TimeToFix_Seconds <= (1 * 60) ) {  
//    TimeToFix_Calculate = TimeToFix_Seconds;                  // 0..60 sec  at 1 sec interval <==> values 0 .. 60 
//  } else if ( TimeToFix_Seconds <= (10 * 60) ) {    
//    TimeToFix_Calculate = 60 + (TimeToFix_Seconds - (1* 60) )/5 ;   // 1..10 min at 5 sec interval  <==> values 60 ..  168 
//  } else if ( TimeToFix_Seconds <= (60 * 60) ) {  
//    TimeToFix_Calculate = 168 + (TimeToFix_Seconds - (10 * 60) )/60 ;   // 10..60 min at 1 min interval <==> values 168 .. 218     
//  } else {
//    TimeToFix_Calculate = 218 + (TimeToFix_Seconds - (60 * 60) )/600 ;    // 1..7:00 hour at 10 min interval <==> values 218 ..254   
//  }
//  if (TimeToFix_Calculate>255) TimeToFix_Calculate = 255 ;                  //  more than 7 hour = 255
//  
//  uint8_t TimeToFix_bin = TimeToFix_Calculate;  // this can contain the values 0..255,
//      
//  myLoraWanData[11] = TimeToFix_bin;
//  #ifdef DEBUG
//  Serial.print(F("TTF=")); Serial.print(TimeToFix_Seconds); Serial.print(F(" sec. bin=")); Serial.print(TimeToFix_bin);
//  #endif
}

//////////////////////////////////////////////////////////
//// GY-BMEP BME280
////////////////////////////////////////////

BME280 bme280;

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
 
  if(!bme280.init()){
    Serial.println("BME280 device error or not found");
  }

  readBME();
}

void readBME(){
  
  float pressure;
  
  //get and print temperatures
  Serial.print(F("BME280 Temp: "));
  Serial.print(bme280.getTemperature());
  Serial.println("C");  //The unit for  Celsius because original arduino don't support speical symbols
  
  //get and print atmospheric pressure data
  Serial.print(F("BME280 Pressure: "));
  Serial.print(pressure = bme280.getPressure());
  Serial.println("Pa");

//  //get and print altitude data
//  Serial.print(F("BME280 Altitude: "));
//  Serial.print(bme280.calcAltitude(pressure));
//  Serial.println("m");

  //get and print humidity data
  Serial.print(F("BME280 Humidity: "));
  Serial.print(bme280.getHumidity());
  Serial.println("%");

}
void put_BME_into_sendbuffer(){
  readBME();

  // TODO
}


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
  #ifdef DEBUGLEVEL2
  Serial.print("  headingDegrees=");
  Serial.print(headingDegrees);
  #endif
  return headingDegrees;
}

///////////////////////////////////////////////
//  arduino init and main
///////////////////////////////////////////
unsigned long device_startTime;
bool has_sent_allready = false; 

void setup() {
  pinMode(LEDPIN, OUTPUT);
  pinMode(btnPin, INPUT_PULLUP);  // 20k internal pullup
  
  delay(2500);     // Give time to the ATMega32u4 port to wake up and be recognized by the OS.
  Serial.begin(115200);   // whether 9600 or 115200; the gps feed shows repeated char and cannot be interpreted, setting high value to release system time
  delay(100);

  Serial.print(F("\nStarting device: ")); Serial.println(DEVADDR); 
  device_startTime = millis();

  gps_init(); 
  lmic_slim_init();
  Compass_init();
  //does not return: BME280_init();   Serial.print(F("  Completed: bme280 init. t=")); Serial.println(millis());
  
  //#ifdef DEBUGLEVEL2
  Serial.print(F("  Init values. t=")); Serial.println(millis());
  //#endif
  put_Volts_and_Temp_into_sendbuffer();
  put_Compass_and_Btn_into_sendbuffer();
  //put_BME_into_sendbuffer();
  doGPS_and_put_values_into_sendbuffer(); 
  //myLoraWanData[22] = MY_GAME_ID << 4 | 1; 
     
  //#ifdef DEBUGLEVEL2
  Serial.print(F("  Send one lorawan message as part of init. t=")); Serial.println(millis());
  //#endif
  last_lora_time = millis();
  last_check_time = millis();
  lmic_slim_init();
  doOneLoraWan();    
  
  #ifdef DEBUGLEVEL2
  Serial.print(F("  Completed: Setup. t=")); Serial.println(millis());
  #endif
}

boolean radioActive = true;  // this name is for radio, not LoraWan
boolean loraWannaBe = false;

void loop() {
  Serial.print(F("\n==== Loop starts. t=")); Serial.println(millis());
  digitalWrite(LEDPIN, !digitalRead(LEDPIN)); 

  // nothing more to do, need to delay till next send time/action time
  while((millis() - last_check_time) < (LORAWAN_TX_INTERVAL_MIN * 1000L)) {
    delay(5000);   
    Serial.print(F("."));
  }
  Serial.println();
    
  ////////// Collect data needed just before sending a LORAWAN update to the world  ///////////
  //Serial.print(F("\nCollect data needed just before sending a LORAWAN update. t=")); Serial.println(millis());
  put_Volts_and_Temp_into_sendbuffer();
  put_Compass_and_Btn_into_sendbuffer();
  //put_BME_into_sendbuffer();
  //myLoraWanData[22] = MY_GAME_ID << 4 | 1; 

  uint32_t LoraWan_Counter = LMIC_getSeqnoUp();  // getCounter zit NIET in LMIC_slim library
  myLoraWanData[23] = LoraWan_Counter >> 8; 
  myLoraWanData[24] = LoraWan_Counter; 

  // Clear gps buffer, or else it will retail old position if no fix is found. If no fix is found we want invalid location so TTNmapper does not get disturbed.
  l_lat = 0; l_lon = 0; l_alt = 678; hdopNumber = 99999;   // the zero position
  doGPS_and_put_values_into_sendbuffer();

  ////////// Now CHECK IF we need to send a LORAWAN update to the world  ///////////
  // switch the LMIC antenna to LoraWan mode
  if ( (millis() - last_lora_time) > ((LORAWAN_TX_INTERVAL_MAX) * 1000L)
      or (l_lat_movement > TXTRIGGER_gps_movement) 
      or (l_lon_movement > TXTRIGGER_gps_movement) ) {
    Serial.print(F("   - - - - About to send one LoraWan. ")); 
    last_lora_time = millis();
    lmic_slim_init();
    doOneLoraWan();    
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
  #ifdef DEBUGLEVEL2
  Serial.println(F("  End of loop. t=")); Serial.println(millis());
  #endif
}



