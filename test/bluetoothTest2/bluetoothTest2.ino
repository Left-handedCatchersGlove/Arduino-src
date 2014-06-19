// 送信プログラム

void setup() {
  Serial.begin(115200);
  pinMode( 2, INPUT );   // 2番ポートを入力に
  pinMode( 13, OUTPUT ); // 13番ポートを出力に
  Serial.println("Start Serial");
}

void loop() {
  if( digitalRead( 2 ) == HIGH ) {
    digitalWrite( 13, HIGH );
    Serial.println("OK!");
  }
  else {
    digitalWrite( 13, LOW );
  }
}
