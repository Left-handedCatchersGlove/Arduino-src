void setup() {
  pinMode( 2, INPUT );   // 入力ピンの設定
  pinMode( 13, OUTPUT ); // 出力ピンの設定
}

void loop() {
  if( digitalRead( 2 ) == HIGH ) {
    digitalWrite( 13, HIGH);
  }
  else {
    digitalWrite( 13, LOW );
  }
}
