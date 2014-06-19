void setup() {
  // 9600
  Serial.begin( 9600 );
}

void loop() {
  int x_val, y_val, z_val;
  
  x_val = analogRead( 0 ); // x軸の値を読み込む、赤色
  y_val = analogRead( 1 ); // y軸の値を読み込む、緑色
  z_val = analogRead( 2 ); // z軸の値を読み込む、黄色
  
  Serial.print( x_val >> 2 );
  Serial.print( y_val >> 2 );
  Serial.print( z_val >> 2 );
  delay( 10 );
}
