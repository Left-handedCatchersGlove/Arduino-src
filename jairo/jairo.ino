void setup()
{
  Serial.begin( 9600 );
  pinMode( 13, OUTPUT );
}

void loop()
{
  digitalWrite( 13, HIGH );
  int x, y;
  
  delay( 10 );
  x = analogRead( 0 );
  y = analogRead( 1 );
  delay( 10 );

  Serial.write( x >> 2 );
  Serial.write( y >> 2 );

  delay( 500 );
}
