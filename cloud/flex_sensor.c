#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "your-ssid";        // Replace with your WiFi SSID
const char* password = "your-password"; // Replace with your WiFi password
const char* api_url = "https://your-api-id.execute-api.region.amazonaws.com/prod/flex"; // Replace with API Gateway URL

void setup() {
  Serial.begin(115200);
  connectToWiFi();
}

void connectToWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected, attempting to reconnect...");
    connectToWiFi();
  }
  
  int flexValue = analogRead(A0);
  Serial.print("Flex Value: ");
  Serial.println(flexValue);
  
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(api_url);
    http.addHeader("Content-Type", "application/json");
    String payload = "{\"flex_value\": " + String(flexValue) + "}";
    int httpCode = http.POST(payload);
    if (httpCode == 200) {
      String response = http.getString();
      Serial.println("Response: " + response);
    } else {
      Serial.println("HTTP Error: " + String(httpCode));
    }
    http.end();
  }
  delay(1000);
}