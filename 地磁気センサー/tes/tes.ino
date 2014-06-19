#include <Wire.h>

union UNION {
        char ch[2];
        int n;
};


void setup() {
  Serial.begin(9600);
  Wire.begin();
}

void loop(){
  union UNION obj;
  Wire.beginTransmission(0x21);
  Wire.write(0x41);
  delay(6);
  Wire.endTransmission(); 
  Wire.requestFrom(0x21,2);
  obj.ch[1] = Wire.read();
  obj.ch[0] = Wire.read();
  Serial.println(obj.n);
}


