// 引脚定义（ESP32）
const int touchPin = T0;    // 触摸引脚，对应GPIO4
const int ledPin = 2;       // 板载LED引脚

// 状态变量
bool ledState = LOW;
bool lastTouchState = false;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;  // 防抖时间 50ms

void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState);
  Serial.begin(115200);  // 串口调试，可选
}

void loop() {
  // 读取触摸值
  int touchValue = touchRead(touchPin);
  bool currentTouchState = (touchValue < 60);  // 可根据实际调整阈值

  // 软件防抖处理
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // 边缘检测：仅在按下瞬间翻转状态
    if (currentTouchState == true && lastTouchState == false) {
      ledState = !ledState;
      digitalWrite(ledPin, ledState);
      Serial.print("LED状态: ");
      Serial.println(ledState ? "ON" : "OFF");
    }

    lastTouchState = currentTouchState;
    lastDebounceTime = millis();
  }
}