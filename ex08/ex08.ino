#include <WiFi.h>

// WiFi 配置（改成你自己的）
//const char* ssid = "你的WiFi名";
const char* ssid = "贝林厄姆";

//const char* password = "你的WiFi密码";
const char* password = "quqichen1";

WiFiServer server(80);

// 硬件引脚
const int touchPin = T0;   // 触摸引脚 (GPIO4)
const int ledPin = 2;      // 板载 LED (可根据实际修改)

// LED PWM 设置
const int pwmFreq = 5000;
const int pwmResolution = 8;   // 0~255，这里我们只用开/关，也可直接用 digitalWrite

// 系统状态变量
bool armed = false;            // 是否布防
bool alarmActive = false;      // 是否正在报警

// 触摸防抖变量
bool lastTouchState = false;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

// LED 闪烁控制（非阻塞）
unsigned long lastBlinkTime = 0;
const unsigned long blinkInterval = 100;  // 100ms 闪烁间隔
bool ledState = false;

void setup() {
  Serial.begin(115200);

  // 初始化 LED PWM（也可改用 pinMode + digitalWrite，这里统一用 PWM）
  ledcAttach(ledPin, pwmFreq, pwmResolution);
  ledcWrite(ledPin, 0);   // 初始熄灭

  // WiFi 连接
  WiFi.begin(ssid, password);
  Serial.print("正在连接 WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("WiFi 已连接，IP 地址: ");
  Serial.println(WiFi.localIP());

  server.begin();
  Serial.println("安防系统已启动");
}

void loop() {
  // ── 1. 处理 Web 客户端请求 ──
  WiFiClient client = server.accept();
  if (client) {
    Serial.println("新客户端连接");
    String request = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        request += c;
        if (request.endsWith("\r\n\r\n")) {
          break;
        }
      }
    }

    // 处理布防/撤防命令
    if (request.startsWith("GET /arm")) {
      armed = true;
      alarmActive = false;       // 布防时清除报警状态
      ledcWrite(ledPin, 0);      // 熄灭 LED
      Serial.println("系统已布防");
      sendResponse(client, "Armed");
    } else if (request.startsWith("GET /disarm")) {
      armed = false;
      alarmActive = false;
      ledcWrite(ledPin, 0);
      Serial.println("系统已撤防");
      sendResponse(client, "Disarmed");
    } else if (request.startsWith("GET /status")) {
      // 可选：返回当前状态（供 AJAX 轮询，本示例未使用）
      String status = armed ? (alarmActive ? "ALARM" : "ARMED") : "DISARMED";
      sendResponse(client, status);
    } else {
      // 主页面 HTML
      String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>安防报警器</title>
  <style>
    body { font-family: Arial; text-align: center; margin-top: 50px; }
    button { padding: 15px 30px; font-size: 18px; margin: 10px; }
    .status { font-size: 24px; margin: 20px; }
    .alarm { color: red; font-weight: bold; }
  </style>
</head>
<body>
  <h1>物联网安防报警器</h1>
  <div class="status">当前状态: <span id="statusText">DISARMED</span></div>
  <button onclick="sendCommand('/arm')">🔒 布防 (Arm)</button>
  <button onclick="sendCommand('/disarm')">🔓 撤防 (Disarm)</button>

  <script>
    function sendCommand(cmd) {
      fetch(cmd)
        .then(response => response.text())
        .then(text => {
          document.getElementById('statusText').innerText = text;
          if (text === 'ALARM') {
            document.getElementById('statusText').className = 'alarm';
          } else {
            document.getElementById('statusText').className = '';
          }
        })
        .catch(error => console.error('Error:', error));
    }

    // 定时轮询状态（每1秒更新一次页面显示）
    setInterval(() => {
      fetch('/status')
        .then(response => response.text())
        .then(text => {
          document.getElementById('statusText').innerText = text;
          if (text === 'ALARM') {
            document.getElementById('statusText').className = 'alarm';
          } else {
            document.getElementById('statusText').className = '';
          }
        });
    }, 1000);
  </script>
</body>
</html>
      )rawliteral";

      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/html; charset=utf-8");
      client.println("Connection: close");
      client.println();
      client.println(html);
    }

    client.stop();
    Serial.println("客户端断开");
  }

  // ── 2. 触摸检测与报警触发 ──
  int touchValue = touchRead(touchPin);
  bool currentTouchState = (touchValue < 60);   // 阈值需校准

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // 边缘检测：从未触摸到触摸的瞬间
    if (currentTouchState && !lastTouchState) {
      // 只有在布防且尚未报警的情况下，才触发报警
      if (armed && !alarmActive) {
        alarmActive = true;
        Serial.println("!!! 触发报警 !!!");
      }
    }
    lastTouchState = currentTouchState;
    lastDebounceTime = millis();
  }

  // ── 3. 报警时的 LED 闪烁（非阻塞） ──
  if (alarmActive) {
    unsigned long now = millis();
    if (now - lastBlinkTime >= blinkInterval) {
      lastBlinkTime = now;
      ledState = !ledState;
      // 用 PWM 全亮或全灭表示闪烁
      ledcWrite(ledPin, ledState ? 255 : 0);
    }
  }
}

// 辅助函数：发送简单响应
void sendResponse(WiFiClient &client, String message) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/plain");
  client.println("Connection: close");
  client.println();
  client.println(message);
}