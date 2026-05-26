// 引脚定义
const int touchPin = T0;   // 触摸引脚，对应 GPIO4
const int ledPin = 2;      // 板载 LED

// PWM 配置
const int pwmFreq = 5000;          // 5kHz，无频闪
const int pwmResolution = 8;       // 8 位分辨率，0~255

// 速度档位对应的亮度步长（越大呼吸越快）
const int stepTable[] = {0, 1, 5, 10};  // 索引 1/2/3 分别对应 1/2/3 档
int speedLevel = 1;                     // 当前档位，初值 1 档（最慢）
int step = stepTable[speedLevel];       // 当前步长

// 呼吸灯状态
int brightness = 0;                     // 当前亮度
int fadeDirection = 1;                  // 1：增亮，-1：变暗
unsigned long lastUpdate = 0;           // 上一次更新 PWM 的时间
const int updateInterval = 10;          // 每 10ms 更新一次亮度（保证流畅）

// 触摸防抖变量
bool lastTouchState = false;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

void setup() {
  Serial.begin(115200);

  // 新版 LEDC 初始化：直接附加引脚，自动分配通道
  // ledcAttach(pin, freq, resolution) 返回通道号（可忽略）
  ledcAttach(ledPin, pwmFreq, pwmResolution);
  ledcWrite(ledPin, brightness);   // 起始熄灭

  Serial.println("多档位触摸调速呼吸灯已启动");
  Serial.print("当前速度档位：");
  Serial.println(speedLevel);
}

void loop() {
  // ── 1. 触摸检测与档位切换 ──
  int touchValue = touchRead(touchPin);
  bool currentTouchState = (touchValue < 60);   // 阈值需实际校准

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // 边缘检测：从未触摸 -> 触摸的瞬间
    if (currentTouchState == true && lastTouchState == false) {
      // 切换档位 1->2->3->1 循环
      speedLevel = (speedLevel % 3) + 1;
      step = stepTable[speedLevel];             // 更新步长
      Serial.print("速度档位切换为：");
      Serial.println(speedLevel);
    }
    lastTouchState = currentTouchState;
    lastDebounceTime = millis();
  }

  // ── 2. 非阻塞呼吸灯效果 ──
  unsigned long now = millis();
  if (now - lastUpdate >= updateInterval) {
    lastUpdate = now;

    // 按当前步长改变亮度
    brightness += (fadeDirection * step);

    // 到达边界时反转方向
    if (brightness >= 255) {
      brightness = 255;
      fadeDirection = -1;   // 开始变暗
    } else if (brightness <= 0) {
      brightness = 0;
      fadeDirection = 1;    // 开始变亮
    }

    // 输出新的 PWM 占空比（新版写法）
    ledcWrite(ledPin, brightness);
  }
}