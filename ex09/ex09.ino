#include <WiFi.h>

// WiFi 配置（改成你自己的）
//const char* ssid = "你的WiFi名";
const char* ssid = "贝林厄姆";

//const char* password = "你的WiFi密码";
const char* password = "quqichen1";

WiFiServer server(80);
const int touchPin = T0;   // 触摸引脚 (GPIO4)

void setup() {
  Serial.begin(115200);

  // 连接 WiFi
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
  Serial.println("Web 仪表盘已启动");
}

void loop() {
  WiFiClient client = server.accept();
  if (client) {
    Serial.println("新客户端连接");
    String request = "";
    unsigned long startTime = millis();
    const unsigned long timeout = 500;

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        request += c;
        if (request.endsWith("\r\n\r\n")) break;
        startTime = millis();
      }
      if (millis() - startTime > timeout) {
        Serial.println("请求超时");
        break;
      }
    }

    // 数据接口：返回触摸传感器数值
    if (request.startsWith("GET /data")) {
      int touchValue = touchRead(touchPin);   // 读取模拟量
      String json = "{\"value\":" + String(touchValue) + "}";
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: application/json");
      client.println("Connection: close");
      client.println();
      client.println(json);
    } else {
      // 主页面 HTML（仪表盘）
      String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>触摸传感器仪表盘</title>
  <style>
    body { font-family: 'Segoe UI', Arial, sans-serif; text-align: center; margin-top: 60px; background: #1a1a2e; color: white; }
    .panel { background: #16213e; border-radius: 20px; padding: 30px; width: 300px; margin: 0 auto; box-shadow: 0 0 20px rgba(0,0,0,0.5); }
    .value { font-size: 72px; font-weight: bold; margin: 20px 0; transition: all 0.2s; }
    .label { font-size: 18px; opacity: 0.8; }
  </style>
</head>
<body>
  <div class="panel">
    <div class="label">触摸传感器数值</div>
    <div class="value" id="sensorValue">--</div>
    <div class="label">手指越近，数值越小</div>
  </div>

  <script>
    function fetchData() {
      fetch('/data')
        .then(response => response.json())
        .then(data => {
          document.getElementById('sensorValue').innerText = data.value;
        })
        .catch(error => console.error('Error:', error));
    }

    // 每 200ms 拉取一次数据
    setInterval(fetchData, 200);
    fetchData(); // 立即执行一次
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