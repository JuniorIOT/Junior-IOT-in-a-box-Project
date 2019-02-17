# Junior-IOT-in-a-box-Project
Code and documentation for Junior-IOT-in-a-box where you can build your own diy sensoring with TTN Lorawan based on Lora32u4

## Inexpensive diy Lora32u4 GPS tracker
- This version has evolved for the IOT in a box, and also to be a match for many applications. https://github.com/JuniorIOT/Junior-IOT-in-a-box-Project
- Iteration 2017 is for the Junior IoT Baloon Challenge february 2018. https://github.com/Kaasfabriek/Junior_Internet_of_Things_2018
- Original iteration in 2016 was created for the Junior IoT Baloon Challenge february 2017 as a basic Lora location transmittor with GPS, arduino nano and rfm95. https://github.com/Kaasfabriek/GPS-Lora-Balloon-rfm95-TinyGPS/tree/master/Balloon-rfm95

This software has been created for edicational reasons. Any commercial usage will need prior agreement with author(s) and compensation for costs and effords.
For any commercial project, consider using non-diy hardware such as the Sodaq One. Your commercial project will benefit from the extensive research and licensing steps as done for such products.

Credits:
- Software creator: Dennis --> dennis.ruigrok@gmail.com
- Workshop maker: Marco --> marco@junioriot.nl

## Important files and folders
- docs - tutorials with pictures
- arduino - you find the arduino code here
- arduino_port - a portable and pre-configured arduino IDE
- libraries - we are using some adjusted versions
- drivers -


## Pin mapping for Lora32u4 in this project
```
    -----------------------------------------------------------------------------

                                        I2C things:
                                                   Groove ==> SCL SDA  V  GND
                                            GY-91 ==> Vin GND SCL SDA
                                          HMC5983 ==> Vin GND SCL SDA
                                          GY-BMEP ==> Vin GND SCL SDA
                                                       ¦   ¦   ¦   ¦
                                                      red blk whi yel
                                                       ¦   ¦   ¦   ¦
                                                               ¦   ¦
                                                               ¦   ¦
                              SDS021==> hole 5V nc GND Rx  Tx  ¦   ¦
                                             ¦     ¦   ¦   ¦   ¦   ¦
                                            red   blk yel blu  ¦   ¦
                                             ¦     ¦   ¦   ¦   ¦   ¦
                                                       ¦   ¦   ¦   ¦
                                                       ¦   ¦   ¦   ¦
  GPS BN-180/BN-200/BN-220 ==> (batt) VCC RX  TX GND   ¦   ¦   ¦   ¦
                                       ¦   ¦   ¦   ¦   ¦   ¦   ¦   ¦
         +---------+                  red gre whi blk  ¦   ¦   ¦   ¦
         ¦   LiPo  ¦                   ¦   ¦   ¦   ¦   ¦   ¦   ¦   ¦
         ¦ 380 mAh ¦                       ¦   ¦       ¦   ¦   ¦   ¦
         ¦protected¦                       ¦   ¦       ¦   ¦   ¦   ¦
         +---------+                       ¦   ¦       ¦   ¦   ¦   ¦
            ¦  ¦                         tx¦ rx¦ Vbat/2¦   ¦   ¦I2c¦        
   +--------¦--¦--- ---+---X---+---X---X---+---+---+---X---X---+---+-------+
   ¦        -  +      BAT EN  5V  13  12  11  10   9   6   5   3   2       ¦
   ¦    (LIPO CONN)           USB LED A11     A10  A9  A7     SCL SDA      ¦
   ¦                           +------+            +----------------+ DIO3 R
   ¦(USB CONN)        LORA32U4 ¦ATMEGA¦            ¦ RFM95 / HDP13  ¦ DIO2 R
   ¦                           ¦ 32U4 ¦            ¦4=rst 7=irq 8=cs¦      ¦
   ¦   (RST BTN)               +------+            +----------------+ ant(0)-
   ¦                   18  19  20  21  22 23  15  16   14  RX  TX          ¦
   ¦  RST 3V3 REF GND  A0  A1  A2  A3  A4 A5 SCK MOSI MISO 0   1 DIO1  ANT +
   +---+--=+===+===+===+=--+---+---X---X---X---R---R---R---+---+---R-------+
           ¦      GND  ¦   ¦   ¦   ¦   ¦   ¦   xxx xxx xxx rx  tx       
           ¦       ¦   ¦   ¦   ¦   ¦   ¦   ¦    SPI-RFM95  serial1      
          ...     ...  ¦   ¦   ¦   ¦   ¦   ¦               ¦   ¦
                   ¦   ¦   ¦   ¦   ¦   ¦   ¦               ¦   ¦
                  GND out  ¦   ¦   ¦   ¦   ¦               ¦   ¦
                  pushBtn  ¦   ¦   ¦   ¦   ¦               ¦   ¦
                           ¦   ¦   ¦   ¦   ¦               ¦   ¦
                           ¦   ¦   ¦   ¦   ¦               ¦   ¦
                  red gre whi  ¦   ¦   ¦   ¦               ¦   ¦
                   ¦   ¦   ¦   ¦   ¦   ¦   ¦               ¦   ¦
     TEMT6000 ==> VCC GND OUT  ¦   ¦   ¦   ¦               ¦   ¦
                               ¦   ¦   ¦   ¦               ¦   ¦
                               ¦   ¦   ¦   ¦               ¦   ¦
                          blk whi gra pur zzz              ¦   ¦
                           ¦   ¦   ¦   ¦   ¦               ¦   ¦
               LeoEQ7 ==> gnd res pul out mic              ¦   ¦      
                                                           ¦   ¦   ¦   ¦
                                            MH-Z19 ==> nc  Tx  Rx Vin GND nc  xx
			 
    GND BAT  
      ¦   ¦
    +-+---+-+
    ¦gnd Vin¦
    ¦step-up¦
    ¦GND 5V ¦
    +-+---+-+

   * R = pins connected to RFM95
   * X = avoid main function on these pins for compatibility with 32u4 Beetle
   * for pushbutton to work, need to enable the internal pull-up
```
## IOT TTN message format
```
    Important: bytes 0 to 8 (nine bytes) are agreed format for TTNmapper.org
      who will ignore any further bytes. They expect SF7 signal, which is our
      default choice also for other reasons.

    -- start with TTNmapper defined format
    byte 0, 1, 2    Latitude   3 bytes, -90 to +90 degr, scaled to 0..16777215
    byte 3, 4, 5    Longitude  3 bytes, -180..+180 degrees, scaled 0..16777215
       note: earth circumfence is 40.075 km; finest data step is 40075*1000/16777215 = 2.5 m
             2.5 m is about the GPS accuracy of 2..3 meters, so data format is efficient
    byte 6, 7       Altitude   2 bytes, in meters. 0..65025 meter
    byte 8          GPS DoP    byte, in 0.1 values. 0..25.5 DoP
       

    -- now our 'internal' values
 9   byte 18         VCC        byte, 50ths, 0 - 5.10 volt -- secret voltmeter
 10   byte 19         CPUtemp    byte, -100 - 155 deg C     -- secret thermometer +/- 10 degrees
 11   byte 20         Vbat       byte, 50ths, 0 - 5.10 volt -- hardwired Lora32u4

    -- A0 switch
 12, 13   byte 9, 10      10 bit analog read from A0 = Pushbutton  0..1023
  14-17  byte 25, 26, 27, 28 DateTime btn on    4 bytes, bits 6 Y-2000, 4 M, 5 D, 5 H, 6 M, 6 S
 18-21   byte 29, 30, 31, 32 DateTime btn off
	
	-- A1 TEMT
22, 23	byte 11, 12     10 bit analog read from A1 = TEMT6000  0..1023
24, 25	    10 bit analog read from A11 
    
	
 x   byte 21         
        0b0000 0000            
          -nnn nnnn Compass    0-120, My compass in 3 degree precision 0..360
                               Value=127: no compass value
          1--- ---- MyBtn#1    bit, is my button pressed
    -- BME280
26, 27    byte 23,24      BME280 Temperature 2 bytes  
27, 29    byte 34, 35     BME280 Moisture   2 bytes, 
30, 31    byte 36, 37     BME280 AirPress   2 bytes,

    -- SDS021	
32, 33    byte 40, 41     SDS021 PPM 2.5    2 bytes, 
34, 35    byte 42, 43     SDS021 PPM 10     2 bytes, 

    -- MH-Z19 
36, 37    byte 38, 39     MH-Z19 CO2        2 bytes, 

    -- LeoEQ7
38-47     Audio averages   10 bytes	
	             Audio 1,     63Hz    10 bit analog read 0..1023
                 Audio 2,    160Hz    10 bit analog read 0..1023
	             Audio 3,    400Hz    10 bit analog read 0..1023
	             Audio 4,  1.000Hz    10 bit analog read 0..1023
	             Audio 5,  2.500Hz    10 bit analog read 0..1023
	             Audio 6,  6.250Hz    10 bit analog read 0..1023
                 Audio 7, 16.000Hz    10 bit analog read 0..1023
                 Audio all            10 bit analog read 0..1023
				
48-57     Audio peak5   10 bytes	
```
