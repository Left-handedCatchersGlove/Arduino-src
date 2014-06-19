#include <Wire.h>

union UNION
{
  char ch[2];
  int n;
};

void setup()
{
  Serial.begin( 9600 );
  Wire.begin();
  
  pinMode( 10, OUTPUT );
  pinMode( 11, OUTPUT );
}

union UNION obj;
int jairo_x, jairo_y;
int val_x, val_y, val_z;

void loop()
{
  digitalWrite( 10, HIGH );
  digitalWrite( 11, HIGH );
  
  delay( 10 );
  
  jairo_x = analogRead( 0 );
  jairo_y = analogRead( 1 );
  
  val_x = analogRead( 2 );
  val_y = analogRead( 3 );
  val_z = analogRead( 4 );
  
  Wire.beginTransmission(0x21);
  Wire.write(0x41);
  Wire.endTransmission(); 
  Wire.requestFrom(0x21,2);
  obj.ch[1] = Wire.read();
  obj.ch[0] = Wire.read();
  
  delay( 10 );
  
  /*Serial.println( jairo_x >> 2 );
  Serial.println( jairo_y >> 2 );
  Serial.println( val_x >> 2 );
  Serial.println( val_y >> 2 );
  Serial.println( val_z >> 2 );
  Serial.println( obj.n >> 2 );
  Serial.println("");*/
  
  // jairo censor
  Serial.write( jairo_x >> 2 );
  Serial.write( jairo_y >> 2 );
  // censor
  Serial.write( val_x >> 2 );
  Serial.write( val_y >> 2 );
  Serial.write( val_z >> 2 );
  // direction censor
  Serial.write( obj.n >> 2 );
  
  delay( 20 );
}
