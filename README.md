# Junior-IOT-in-a-box-Project
Code and documentation for Junior-IOT-in-a-box where you can build your own diy sensoring with TTN Lorawan based on Lora32u4


previous version: 
Inexpensive low-quality Lora32u4 GPS tracker for #juniorIOTchallenge and JuniorIOTinabox

## Inexpensive low-quality Lora32u4 GPS tracker
  this version has evolved for the IOT in a box, and also to be a match for many applications
    https://github.com/JuniorIOT/Junior-IOT-in-a-box-Project
  iteration 2017 is for the Junior IoT Baloon Challenge february 2018
    https://github.com/Kaasfabriek/Junior_Internet_of_Things_2018
  original iteration in 2016 was created for the Junior IoT Baloon Challenge february 2017
    so for a basic Lora location transmittor with GPS, arduino nano and rfm95 see 
    https://github.com/Kaasfabriek/GPS-Lora-Balloon-rfm95-TinyGPS/tree/master/Balloon-rfm95

Credits:
- Software plakker: Dennis --> dennis.ruigrok@gmail.com
- Educatie kletser: Marco --> marco@junioriot.nl
- Regie en inspiratie: marco@marcovanschagen.nl
- Junior IOT Challenges: marco@dataschrift.nl

## Important



## Important files and folders
- arduino folder
- docs 
- libraries - we are using some adjusted versions
- drivers


## Pin mapping for Lora32u4 in this project
```
    -----------------------------------------------------------------------------

                                        I2C things:
                                                   Groove ==> SCL SDA  V  GND
                                            GY-91 ==> Vin GND SCL SDA
                                          HMC5983 ==> Vin GND SCL SDA
                                          GY-BMEP ==> Vin GND SCL SDA
                                                       ¦   ¦   ¦   ¦
  GPS BN-180/BN-200/BN-220 ==> (batt) VCC RX  TX GND   ¦   ¦   ¦   ¦
                                       ¦   ¦   ¦   ¦   ¦   ¦   ¦   ¦
       +---------+                    red gre whi bla  ¦   ¦   ¦   ¦
       ¦   LiPo  ¦                     ¦   ¦   ¦   ¦   ¦   ¦   ¦   ¦
       ¦ 380 mAh ¦                         ¦   ¦       ¦   ¦   ¦   ¦
       ¦protected¦                         ¦   ¦       ¦   ¦   ¦   ¦
       +---------+                         ¦   ¦       ¦   ¦   ¦   ¦
            ¦  ¦                         tx¦ rx¦ Vbat/2¦   ¦   ¦I2c¦        
   +--------¦--¦--- ---+---X---+---X---X---+---+---+---X---X---+---+-------+ 
   ¦        -  +      BAT EN  5V  13  12  11  10   9   6   5   3   2       ¦ 
   ¦    (LIPO CONN)              LED  A1      A10  A9  A7     SCL SDA      ¦ 
   ¦                           +------+            +----------------+ DIO3 R 
   ¦(USB CONN)        LORA32U4 ¦ATMEGA¦            ¦ RFM95 / HDP13  ¦ DIO2 R 
   ¦                           ¦ 32U4 ¦            ¦4=rst 7=irq 8=cs¦      ¦ 
   ¦   (RST BTN)               +------+            +----------------+ ant(0)-
   ¦                                          15  16                       ¦ 
   ¦  RST 3V3 REF GND  A0  A1  A2  A3  A4 A5 SCK MOSI MISO 0   1 DIO1  ANT +
   +---+--=+===+===+===+=--+---+---X---X---X---R---R---R---+---+---R-------+
           ¦      GND  ¦   ?   ?   ¦           xxx xxx xxx  rx tx       
           ¦       ¦   ¦   ¦   ¦   ¦           SPI-RFM95  serial1      
          ...     ...  ¦   ¦   ¦   ¦                       ¦   ¦ 
                   ¦   ¦   ¦   ¦   ¦                       ¦   ¦
                  GND out  ¦   ¦   ¦                       ¦   ¦
                  pushBtn  ¦   ¦   ¦                       ¦   ¦
                           ¦   ¦   ¦                       ¦   ¦   ¦   ¦ 
                           ¦   ¦   ¦        MH-Z19 ==> nc  Tx  Rx Vin GND nc  xx
                 ¦     ¦   ¦   ¦   ¦                       
                red   bla yel blu  ¦                       
                 ¦     ¦   ¦   ¦   ¦                       
  SDS021==> hole 5V nc GND Rx  Tx  ¦                       
                                   ¦                       
                                  whi gre red                      
                                   ¦   ¦   ¦                       
                     TEMT6000 ==> OUT GND VCC                                          
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
    
    -- phased out
    byte 9, 10, 11  
    byte 12, 13, 14 
    byte 15, 16     
    byte 17            
                               
    -- now our 'regular' values
    byte 18         VCC        byte, 50ths, 0 - 5.10 volt -- secret voltmeter
    byte 19         CPUtemp    byte, -100 - 155 deg C     -- secret thermometer +/- 10 degrees
    byte 20         Vbat       byte, 50ths, 0 - 5.10 volt -- hardwired Lora32u4
                               
    byte 21         
        0b0000 0000            
          -nnn nnnn Compass    0-120, My compass in 3 degree precision 0..360
                               Value=127: no compass value
          1--- ---- MyBtn#1    bit, is my button pressed
          
    byte 22         phased out
    byte 23,24      Temperature 2 bytes  (was: counter, 2 bytes)
	                     
    -- phased out
    byte 25         
    byte 26, 27, 28 
    byte 29, 30, 31 
    byte 32         
    byte 33         
		
    -- OPTIONAL set environmental sensors values ((leave 00 if not used)
    byte 34, 35     Moisture   2 bytes, AD measurement directly from AD port
    byte 36, 37     AirPress   2 bytes, AD measurement directly from AD port
    byte 38, 39     CO2        2 bytes, AD measurement directly from AD port
    byte 40, 41     PPM 2.5    2 bytes, AD measurement directly from AD port put_PM_into_sendbuffer
    byte 42, 43     PPM 10     2 bytes, AD measurement directly from AD port
    byte 44, 45     Audio 1    2 bytes
    byte 46, 47     Audio 2    2 bytes

```

