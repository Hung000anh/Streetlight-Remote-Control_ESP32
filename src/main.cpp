#include <WiFi.h>
#include <PubSubClient.h>

// Cấu hình Wi-Fi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Thông tin ThingsBoard
const char* thingsboardServer = "thingsboard.cloud";  // Địa chỉ server ThingsBoard
const int thingsboardPort = 1883;                      // Port MQTT (mặc định là 1883)
const char* accessToken = "kYUfEw6B3rNghi92RkJa";      // Access Token của thiết bị trên ThingsBoard

WiFiClient espClient;
PubSubClient client(espClient);

// Chân GPIO kết nối Relay/LED
const int relayPin = 23;

// Hàm callback nhận các lệnh RPC từ ThingsBoard
void callback(char* topic, byte* payload, unsigned int length) {
  // Chuyển payload thành chuỗi
  String command = "";
  for (unsigned int i = 0; i < length; i++) {
    command += (char)payload[i];
  }
  Serial.println("Lệnh nhận được: " + command);

  if (command.indexOf("\"method\":\"setValue\"") >= 0) {
  if (command.indexOf("\"params\":\"ON\"") >= 0) {
    digitalWrite(relayPin, HIGH);
  } else if (command.indexOf("\"params\":\"OFF\"") >= 0) {
    digitalWrite(relayPin, LOW);
  }

  // Gửi lại trạng thái về ThingsBoard
  String status = digitalRead(relayPin) == HIGH ? "ON" : "OFF";
  String payload = "{\"status\":\"" + status + "\"}";
  client.publish("v1/devices/me/telemetry", payload.c_str());
  Serial.println("Đã gửi telemetry: " + payload);
}
}

void setup() {
  // Khởi tạo Serial
  Serial.begin(115200);

  // Kết nối Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Đang kết nối Wi-Fi...");
  }
  Serial.println("Đã kết nối Wi-Fi");

  // Kết nối tới ThingsBoard
  client.setServer(thingsboardServer, thingsboardPort);
  client.setCallback(callback);
  // Cài đặt chân GPIO cho Relay/LED
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);  // Đảm bảo Relay tắt khi khởi động
}

void reconnect() {
  // Kết nối lại nếu mất kết nối với ThingsBoard
  while (!client.connected()) {
    Serial.print("Đang kết nối ThingsBoard...");
    if (client.connect("ESP32Client", accessToken, NULL)) {
      Serial.println("Đã kết nối");
      client.subscribe("v1/devices/me/rpc/request/+");  // Subscribe để nhận lệnh RPC từ ThingsBoard
    } else {
      Serial.print("Lỗi kết nối, mã lỗi: ");
      Serial.println(client.state());
      delay(5000);
    }
  }
}

void loop() {
  // Nếu mất kết nối, reconnect lại
  if (!client.connected()) {
    reconnect();
  }
  
  client.loop();
}


