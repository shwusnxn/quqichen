// 双通道 PWM 警车双闪灯效
// LED_A：GPIO4，LED_B：GPIO5

const int ledPinA = 4;
const int ledPinB = 5;

// PWM 参数
const int pwmFreq = 5000;
const int pwmResolution = 8;   // 0~255

int brightnessA = 0;            // LED A 当前亮度
int fadeDirection = 1;          // 1：增亮，-1：变暗
unsigned long lastUpdate = 0;
const int updateInterval = 8;   // 每 8ms 更新一次，渐变流畅

void setup() {
  Serial.begin(115200);

  // 初始化两个 PWM 通道（新版 API 自动分配）
  ledcAttach(ledPinA, pwmFreq, pwmResolution);
  ledcAttach(ledPinB, pwmFreq, pwmResolution);

  // 初始状态：A 最暗，B 最亮
  ledcWrite(ledPinA, brightnessA);
  ledcWrite(ledPinB, 255 - brightnessA);
}

void loop() {
  unsigned long now = millis();
  if (now - lastUpdate >= updateInterval) {
    lastUpdate = now;

    // 更新 A 的亮度，B 自动反相
    brightnessA += fadeDirection;

    if (brightnessA >= 255) {
      brightnessA = 255;
      fadeDirection = -1;       // 开始变暗
    } else if (brightnessA <= 0) {
      brightnessA = 0;
      fadeDirection = 1;        // 开始变亮
    }

    // 写入两个通道
    ledcWrite(ledPinA, brightnessA);
    ledcWrite(ledPinB, 255 - brightnessA);
  }
}