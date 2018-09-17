
// test for SDS021 Particle Matter sensor
//
//       THIS TEST PROGRAM NEEDS DEBUGGING - NOT WORKING
//

/*
  Reading bytes from SDS021 Air Quality PM2.5/10 Particle Sensor
  By: Nathan Seidle
  SparkFun Electronics
  Date: May 8th, 2017
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  This example shows how to read the PM2.5 and PM10 readings from the sensor
*/

#define pin_PM_TXD_rx A2  // SDS021 TXD is connected to A2
#define pin_PM_RXD_tx A1

#include <SoftwareSerial.h>

SoftwareSerial pm_serial(A1, A2); // RX, TX

float pm25; //2.5um particles detected in ug/m3
float pm10; //10um particles detected in ug/m3
unsigned int deviceID; //Two byte unique ID set by factory

//Scans for incoming packet. Times out after 1500 miliseconds
boolean pm_doOneMeasure(void)
{
  //Spin until we hear meassage header byte
  long startTime = millis();

  while (1)
  {
    while (!pm_serial.available())
    {
      delay(1);
      if (millis() - startTime > 1500) return (false); //Timeout error
    }

    if (pm_serial.read() == 0xAA) break; //We have the message header
  }

  //Read the next 9 bytes
  byte sensorValue[10];
  for (byte spot = 1 ; spot < 10 ; spot++)
  {
    startTime = millis();
    while (!pm_serial.available())
    {
      delay(1);
      if (millis() - startTime > 1500) return (false); //Timeout error
    }

    sensorValue[spot] = pm_serial.read();
  }

  //Check CRC
  byte crc = 0;
  for (byte x = 2 ; x < 8 ; x++) //DATA1+DATA2+...+DATA6
    crc += sensorValue[x];
  if (crc != sensorValue[8])
    return (false); //CRC error

  if (sensorValue[1] == 0xC0) //This is just a normal reading
  {
    //Update the global variables
    pm25 = ((float)sensorValue[3] * 256 + sensorValue[2]) / 10;
    pm10 = ((float)sensorValue[5] * 256 + sensorValue[4]) / 10;
    deviceID = sensorValue[6] * 256 + sensorValue[7];
  }
  else if (sensorValue[1] == 0xC5) //Response to command
  {
    Serial.println("  pm Response to command found");

    if (sensorValue[2] == 7) //Firmware response
    {
      Serial.print("  pm Firmware version Y/M/D: ");
      Serial.print(sensorValue[3]);
      Serial.print("/");
      Serial.print(sensorValue[4]);
      Serial.print("/");
      Serial.print(sensorValue[5]);
      Serial.println();
    }
    else if (sensorValue[2] == 6) //Query/Set work and sleep modes
    {
      if (sensorValue[3] == 1) //Response to set mode
      {
        Serial.print("  pm Sensor, status going to ");
        if (sensorValue[4] == 0) Serial.println("sleep");
        else if (sensorValue[4] == 1) Serial.println("work");
      }
    }
  }

//  Serial.print("  pm Raw data:");
//  for (int x = 1 ; x < 10 ; x++)
//  {
//    Serial.print(" ");
//    Serial.print(x);
//    Serial.print(":0x");
//    Serial.print(sensorValue[x], HEX);
//  }
//  Serial.println();

  return (true); //We've got a good reading!
}


//Send a command packet to the module
//Requires the command number and two setting bytes
//Calculates CRC and attaches all header/ender bytes
//Assumes you are only talking to one sensor
void pm_sendCommand(byte commandNumber, byte dataByte2, byte dataByte3)
{
  byte packet[19]; //It's 19 bytes big
  packet[0] = 0xAA; //Message header
  packet[1] = 0xB4; //Packet type = Command
  packet[2] = commandNumber; //Type of command we want to do
  packet[3] = dataByte2; //These are specific to each command
  packet[4] = dataByte3;

  for (byte x = 5; x < 15 ; x++)
    packet[x] = 0; //Reserved bytes

  packet[15] = 0xFF; //Talk to whatever sensor we are connected to. No specific device ID.
  packet[16] = 0xFF; //Talk to whatever sensor we are connected to. No specific device ID.

  //packet[15] = 0xA4; //Talk to specific sensor
  //packet[16] = 0xE6; //Talk to specific sensor

  //Caculate CRC
  byte crc = 0;
  for (byte x = 2 ; x < 17 ; x++)
    crc += packet[x];

  packet[17] = crc;
  packet[18] = 0xAB; //Tail

  //Display the contents of the command packet for debugging
  /*Serial.print("Command packet:");
    for(int x = 0 ; x < 19 ; x++)
    {
    Serial.print(" ");
    Serial.print(x);
    Serial.print(":0x");
    Serial.print(packet[x], HEX);
    }
    Serial.println();*/

  //The sensor seems to fail to respond to the first 2 or 3 times we send a command
  //Hardware serial doesn't have this issue but software serial does.
  //Sending 10 throw away characters at it gets the units talking correctly
  for (byte x = 0 ; x < 10 ; x++)
    pm_serial.write('!'); //Just get the software serial working

  //Send command packet
  for (byte x = 0 ; x < 19 ; x++)
    pm_serial.write(packet[x]);

  //Now look for response
  pm_doOneMeasure();
}

//Print the firmware version
void pm_getFirmwareVersion(void)
{
  pm_sendCommand(7, 0, 0); //Command number is 7, no databytes
}

//Tell the module to go to sleep
void pm_goToSleep(void)
{
  pm_sendCommand(6, 1, 0); //Command number is 6, set mode = 1, sleep = 0
}

//Tell module to start working!
void pm_wakeUp(void)
{
  pm_sendCommand(6, 1, 1); //Command number is 6, set mode = 1, work = 1
}


void setup_pm()
{
  Serial.println("  pm Setup sensor");
  pm_serial.begin(9600);          //SDS021 reports at 1Hz at 9600bps
}

void pm_measure()
{
  if (pm_doOneMeasure())
  {
    Serial.print("  pm Particle Matter [2.5]: ");
    Serial.print(pm25, 1);
    Serial.print("ug/m3 [10]: ");
    Serial.print(pm10, 1);
    Serial.print("ug/m3");
    Serial.println();
  }
  else
  {
    Serial.println("  pm No PM sensor found or timed-out.");
  }
}
//
//void put_PM_into_sendbuffer() {
//  Serial.print(F("put_PM_into_sendbuffer started. milis=")); Serial.println(millis());
//  // pm25_bin and pm10_bin are 2 byte values
//  //   byte 40, 41     PPM 2.5    2 bytes, AD measurement directly from AD port put_PM_into_sendbuffer
//  //   byte 42, 43     PPM 10     2 bytes, AD measurement directly from AD port
//  
//  myLoraWanData[40] = ( pm25_bin >> 8 ) & 0xFF;
//  myLoraWanData[41] = pm25_bin & 0xFF;
//  
//  myLoraWanData[42] = ( pm10_bin >> 8 ) & 0xFF;
//  myLoraWanData[43] = pm10_bin & 0xFF;
//  
//  #ifdef DEBUG
//  Serial.print(F("  pm25_bin=")); Serial.print(pm25_bin); Serial.print(F("  pm10_bin=")); Serial.println(pm10_bin);
//  #endif
//}


  
void setup() {
  Serial.begin(115200);
  delay(1500);
  
  setup_pm();
  
  Serial.println(F("\nStarting SDS021 test.")); 
}

void loop() {

  pm_measure();
  
  delay(1000);
}

