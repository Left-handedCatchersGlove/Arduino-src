void setup() {
  // 9600
  Serial.begin( 9600 );
  pinMode( 13, OUTPUT );
}

void loop() {
  digitalWrite( 13, HIGH );
  int x_val, y_val, z_val;
  
  delay( 10 );
  x_val = analogRead( 0 );
  y_val = analogRead( 1 );
  z_val = analogRead( 2 );
  delay( 10 );
  
  Serial.write( x_val >> 2 );
  Serial.write( y_val >> 2 );
  Serial.write( z_val >> 2 );
  
  //Serial.write( 0x30 );
  //Serial.write( 0x20 );
  //Serial.write( 0x10 );
  //Serial.println( x_val >> 2 );
  //Serial.println( y_val >> 2 );
  //Serial.println( z_val >> 2 );
  //Serial.println("");
  delay( 100 );
}

