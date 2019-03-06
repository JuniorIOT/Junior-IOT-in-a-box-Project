
///////////////////////////////////////////////
//  some other measurements
///////////////////////////////////////////
void print_encoded_dt(unsigned long); //defined elsewhere 

#define VBATPIN A9
#define LEDPIN 13 
unsigned long datetime_BtnOn = 0;
unsigned long datetime_BtnOff = 0;
int myBtn_prev=-1;

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

double GetTemp(void) { 
  //return GetTemp_Atmega32();
  return GetTemp_Atmega32u4();
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

void put_AnalogReads_into_sendbuffer() {

  // A0 = BTN
  // A1 = TEMT6000
  // A2
  // A3
  // A4
  // A5
  
  int A0Value = analogRead(A0);
  delay(1);
  int A1Value = analogRead(A1);
//  delay(1);
//  int A2Value = analogRead(A2);
//  delay(1);
//  int A3Value = analogRead(A3);
//  delay(1);
//  int A4Value = analogRead(A4);
//  delay(1);
//  int A5Value = analogRead(A5);
  
  // now add a bit for BTN 
  uint8_t myBtn = 0;  
  if (A0Value>100) myBtn=1;
  if(myBtn_prev==-1) myBtn_prev=myBtn;

  // did Btn change?
  if(myBtn_prev != myBtn) { 
    if(myBtn==0) {
      Serial.println(F("  (BTN switched off) "));
      datetime_BtnOff = datetime_gps;
    }
    if(myBtn==1) {
      Serial.println(F("  (BTN switched ON) "));
      datetime_BtnOn = datetime_gps;
    }
    myBtn_prev=myBtn;
  }
  
  Serial.print(F("  BTN A0Value="));
  Serial.print( A0Value);
  Serial.print(F(" myBtn="));
  Serial.print( myBtn);
  Serial.print(F(" myBtn_prev="));
  Serial.println( myBtn_prev);
  Serial.print(F("    datetime_BtnOn="));
  Serial.print( datetime_BtnOn);
  print_encoded_dt(datetime_BtnOn);
  Serial.print(F("    datetime_BtnOff="));
  Serial.print( datetime_BtnOff);
  print_encoded_dt(datetime_BtnOff);
  Serial.print(F("    datetime_gps="));
  print_encoded_dt( datetime_gps);
  
  Serial.print(F("  TEMT A1Value="));
  Serial.print( A1Value);
//  Serial.print(F("  A2Value="));
//  Serial.print( A2Value);
//  Serial.print(F("  A3Value="));
//  Serial.print( A3Value);
//  Serial.print(F("  A4Value="));
//  Serial.print( A4Value);
//  Serial.print(F("  A5Value="));
//  Serial.print( A5Value);
//  Serial.println();


  myLoraWanData[internals_sendbufferStartByte + 3] = A0Value >> 8;
  myLoraWanData[internals_sendbufferStartByte + 4] = A0Value;
  myLoraWanData[internals_sendbufferStartByte + 5] = A1Value >> 8;
  myLoraWanData[internals_sendbufferStartByte + 6] = A1Value;
  //                                        waar is A11? 2 bytes
              
  //    byte 25, 26, 27, 28 DateTime btn on    4 bytes, bits 6 Y-2000, 4 M, 5 D, 5 H, 6 M, 6 S
  myLoraWanData[internals_sendbufferStartByte + 9] = datetime_BtnOn >>24; 
  myLoraWanData[internals_sendbufferStartByte + 10] = datetime_BtnOn >>16; 
  myLoraWanData[internals_sendbufferStartByte + 11] = datetime_BtnOn >>8; 
  myLoraWanData[internals_sendbufferStartByte + 12] = datetime_BtnOn; 

  //    byte 25, 26, 27, 28 DateTime btn on    4 bytes, bits 6 Y-2000, 4 M, 5 D, 5 H, 6 M, 6 S
  myLoraWanData[internals_sendbufferStartByte + 13] = datetime_BtnOff >>24; 
  myLoraWanData[internals_sendbufferStartByte + 14] = datetime_BtnOff >>16; 
  myLoraWanData[internals_sendbufferStartByte + 15] = datetime_BtnOff >>8; 
  myLoraWanData[internals_sendbufferStartByte + 16] = datetime_BtnOff; 
  
}

void put_Volts_and_Temp_into_sendbuffer() {

  long vcc = readVccCPU();
  //long vcc = 0;
  uint8_t vcc_bin = vcc/20 ;  // rescale 0-5100 milli volt into 0 - 255 values and make sure it is not bigger than one byte
  #ifdef SHOW_DEBUG
  Serial.print(F("  Vcc=")); Serial.print(vcc); Serial.print(F(" mV. bin=")); Serial.print(vcc_bin);
  #endif
  
  double cpuTemp = GetTemp();
  uint8_t cpuTemp_bin = cpuTemp + 100;   // rescale -100 to 155 into 0 - 255 values and make sure it is not bigger than one byte
  #ifdef SHOW_DEBUG
  Serial.print(F("  CpuTemp=")); Serial.print(cpuTemp); Serial.print(F(" bin=")); Serial.print(cpuTemp_bin);
  #endif
  
  long vbat = readVbat();
  uint8_t vbat_bin = vbat/20 ;  // rescale 0-5100 milli volt into 0 - 255 values and make sure it is not bigger than one byte
  #ifdef SHOW_DEBUG
  Serial.print(F("  Vbat=")); Serial.print(vbat); Serial.print(F(" mV. bin=")); Serial.print(vbat_bin); Serial.println();
  #endif 
  
  myLoraWanData[internals_sendbufferStartByte + 0] = vcc_bin;
  myLoraWanData[internals_sendbufferStartByte + 1] = cpuTemp_bin;
  myLoraWanData[internals_sendbufferStartByte + 2] = vbat_bin;

}

//------------------------------

void internals_measure() {
  put_Volts_and_Temp_into_sendbuffer();
  put_AnalogReads_into_sendbuffer();
}


void internals_init () {
  pinMode(LEDPIN, OUTPUT);
  pinMode(A0, INPUT_PULLUP);  // 20k internal pullup
  pinMode(A3, INPUT_PULLUP);  // 20k internal pullup
  pinMode(A4, INPUT_PULLUP);  // 20k internal pullup
  pinMode(A5, INPUT_PULLUP);  // 20k internal pullup
  

  // as part of init, do one measurement to load values into send buffer
  internals_measure();
}
