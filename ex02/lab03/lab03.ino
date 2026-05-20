// 定义LED引脚
const int ledPin = 2;  

// 设置PWM属性
const unsigned long interval = 500;  // 500ms = 0.5秒，即半个周期
const int resolution = 8;       // 分辨率 8位 (0-255)


unsigned long previousMillis = 0;
bool ledState = false;

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState);

}

void loop() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;  // 保存当前时间
    
    ledState = !ledState;            // 翻转LED状态
    digitalWrite(ledPin, ledState);  // 更新LED
  }
  
  Serial.println("Breathing cycle completed");
}