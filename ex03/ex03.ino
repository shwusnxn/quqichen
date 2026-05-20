const int ledPin = 2;

unsigned long startTime;

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
}

void loop() {
  // ===== 第一个 S：3个短闪 =====
  for(int i = 0; i < 3; i++) {
    // 亮 200ms
    startTime = millis();
    digitalWrite(ledPin, HIGH);
    Serial.println("LED ON - S dot");
    while(millis() - startTime < 200);
    
    // 灭 200ms
    startTime = millis();
    digitalWrite(ledPin, LOW);
    Serial.println("LED OFF");
    while(millis() - startTime < 200);
  }
  
  // 字母间隔 500ms
  startTime = millis();
  while(millis() - startTime < 500);
  
  // ===== O：3个长闪 =====
  for(int i = 0; i < 3; i++) {
    // 亮 600ms
    startTime = millis();
    digitalWrite(ledPin, HIGH);
    Serial.println("LED ON - O dash");
    while(millis() - startTime < 600);
    
    // 灭 200ms
    startTime = millis();
    digitalWrite(ledPin, LOW);
    Serial.println("LED OFF");
    while(millis() - startTime < 200);
  }
  
  // 字母间隔 500ms
  startTime = millis();
  while(millis() - startTime < 500);
  
  // ===== 第二个 S：3个短闪 =====
  for(int i = 0; i < 3; i++) {
    // 亮 200ms
    startTime = millis();
    digitalWrite(ledPin, HIGH);
    Serial.println("LED ON - S dot");
    while(millis() - startTime < 200);
    
    // 灭 200ms
    startTime = millis();
    digitalWrite(ledPin, LOW);
    Serial.println("LED OFF");
    while(millis() - startTime < 200);
  }
  

  startTime = millis();
  while(millis() - startTime < 4000);
}