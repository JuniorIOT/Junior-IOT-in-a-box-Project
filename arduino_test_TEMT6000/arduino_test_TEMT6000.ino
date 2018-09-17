
// test for TEMT6000 on Analog pin A3


//////////////////////////////////////////////////////////
//// TEMT6000 - light sensor
////////////////////////////////////////////


  // A3 = TEMT6000

  
void setup() {
  Serial.begin(115200);
  delay(1500);
  
  pinMode(A3, INPUT_PULLUP);  // 20k internal pullup
  
  Serial.println(F("\nStarting TEMT6000 test.")); 
}

void loop() {

  int A3Value = analogRead(A3);
  
  Serial.print(F("  TEMT A3Value="));
  Serial.print( A3Value);
  Serial.println();  
  
  delay(1000);
}

