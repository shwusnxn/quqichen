#include <WiFi.h>

// WiFi 配置
//const char* ssid = "你的WiFi名";
const char* ssid = "iQOO Z9 Turbo";

//const char* password = "你的WiFi密码";
const char* password = "1564922632";
WiFiServer server(80);

const int ledPin = 2;          // 板载 LED，可根据实际修改
const int pwmFreq = 5000;
const int pwmResolution = 8;   // 0~255

void setup() {
  Serial.begin(115200);

  ledcAttach(ledPin, pwmFreq, pwmResolution);
  ledcWrite(ledPin, 0);

  // ---------- 新增：WiFi 扫描 ----------
  Serial.println("开始扫描附近 WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  Serial.println("扫描完成");
  if (n == 0) {
    Serial.println("未发现任何 WiFi 网络");
  } else {
    Serial.print("发现 ");
    Serial.print(n);
    Serial.println(" 个网络：");
    for (int i = 0; i < n; i++) {
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (信号强度: ");
      Serial.print(WiFi.RSSI(i));
      Serial.println(")");
      delay(10);
    }
  }
  Serial.println("-------------");
  // ---------- 扫描结束 ----------

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
}

void loop() {
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

    // 处理调光请求
    if (request.startsWith("GET /set?value=")) {
      int valueStart = request.indexOf("value=") + 6;
      int valueEnd = request.indexOf(" ", valueStart);
      String valueStr = request.substring(valueStart, valueEnd);
      int brightness = valueStr.toInt();
      brightness = constrain(brightness, 0, 255);
      ledcWrite(ledPin, brightness);
      Serial.print("设置亮度: ");
      Serial.println(brightness);

      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/plain");
      client.println("Connection: close");
      client.println();
      client.println("OK");
    } else {
      // 主页 HTML
      String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 无极调光器</title>
  <style>
    body { font-family: Arial; text-align: center; margin-top: 50px; }
    input[type=range] { width: 80%; margin: 20px; }
  </style>
</head>
<body>
  <h1>ESP32 无极调光器</h1>
  <input type="range" id="brightnessSlider" min="0" max="255" value="0" oninput="updateBrightness(this.value)">
  <p>当前亮度: <span id="brightnessValue">0</span></p>
  <script>
    function updateBrightness(value) {
      document.getElementById('brightnessValue').innerText = value;
      fetch('/set?value=' + value)
        .then(response => response.text())
        .catch(error => console.error('Error:', error));
    }
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
}