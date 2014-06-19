// 受信用のプログラム

void setup() {
  Serial.begin( 115200 ); // シリアル通信のボーレート決定
  pinMode( 13 , OUTPUT );
}

void loop() {
  if( Serial.available() > 0 ) { // IDEからデータを受信する  
    char c = Serial.read();
    if( c == 'n' ) {
      digitalWrite(13,LOW);
    }
    else if( c == 'f' ) {
      digitalWrite(13,HIGH);
    }
  } 
}
