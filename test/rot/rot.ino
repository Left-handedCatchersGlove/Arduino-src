void setup() {
  Serial.begin( 9600 );
}

void loop() {
  int x_val, y_val, z_val;
  
  x_val = analogRead( 0 ); // x軸の値を読み込む、赤色
  y_val = analogRead( 1 ); // y軸の値を読み込む、緑色
  z_val = analogRead( 2 ); // z軸の値を読み込む、黄色
  
  analogWrite( 3, x_val >> 2 );
  analogWrite( 5, y_val >> 2 );
  analogWrite( 6, z_val >> 2 );
  
  Serial.print( " x = " );
  Serial.println( x_val );
  Serial.print( " y = " );
  Serial.println( y_val );
  Serial.print( " z = " );
  Serial.println( z_val );
  Serial.println( "-------" );
  delay( 500 );
}
