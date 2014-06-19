int val;

void setup() {
  pinMode( 13, OUTPUT );
  Serial.begin( 9600 );
}

void loop() {
  val = analogRead( 0 ); // 赤外線センサで読み込んだ値
  Serial.println( val );
  val = val >> 2;        // 2bitシフト
  analogWrite( 3, val ); // 結果をPWMPinの３番に反映
}
