#include <ESP8266WiFiAdapter.h>
#include <StoredCredentials.h>
#include <ArduinoJson.h>

// extern initialization section
char*   WIFI_AP_SSID        = "ESP8266";
char*   WIFI_AP_PASSWORD    = "";

char*   MQTT_SERVER         = "fill in with your MQTT broker data";
int     MQTT_PORT           = 12345;
char*   MQTT_USER           = "fill in with your MQTT broker data";
char*   MQTT_PASSWORD       = "fill in with your MQTT broker data";
char*   MQTT_MODULE_ID      = "ESP8266";
char*   MQTT_CLIENT_TOPIC   = "fill in with your MQTT broker data";

int     IDLE_PIN            = 2;
int     CLIENT_PIN          = 4;
int     SERVER_PIN          = 5;
int     BUTTON_PIN          = 13;
int     SWITCH_PIN          = 14;

const char* CONTENT_TYPE_JSON = "application/json";

void mqttClientCallback(char* topic, byte* payload, unsigned int length) {
  byte target[length];
  for (int i = 0; i < length; ++i)
    target[i] = payload[i];
  String request((char*) target);
  if (request == "ON") {
    digitalWrite(SWITCH_PIN, HIGH);
  } else if (request == "OFF") {
    digitalWrite(SWITCH_PIN, LOW);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(SWITCH_PIN, OUTPUT);
  attachInterrupt(BUTTON_PIN, startServer, RISING);
  WiFiAdapter.changeMode(CLIENT);
  setupRequestHandler();
}

void setupRequestHandler() {
  HttpRequestHandler.on("/ping", HTTP_GET, [](){
    HttpRequestHandler.send(200, CONTENT_TYPE_JSON, "{status: \"OK\"}");
  });
  HttpRequestHandler.on("/networks", HTTP_GET, [](){
    char* body = scanNetworks();
    HttpRequestHandler.send(200, CONTENT_TYPE_JSON, body);
  });
  HttpRequestHandler.on("/configuration/network", HTTP_POST, [](){
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(HttpRequestHandler.arg("plain"));
    String ssid = root["ssid"];
    String password = root["password"];
    StoredCredentials.create(ssid, password);
    HttpRequestHandler.send(200, CONTENT_TYPE_JSON, "{status: \"OK\"}");
  });
}

char* scanNetworks() {
  // create json object
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  JsonArray& networks = root.createNestedArray("networks");
  // scan networks - returns number of networks discovered
  int networksAvailable = WiFi.scanNetworks();
  for (int i = 0; i < networksAvailable; i++) {
    JsonObject& networkInfo = networks.createNestedObject();
    networkInfo["ssid"] = WiFi.SSID(i);
    networkInfo["encryption"] = encryptionTypeStr(WiFi.encryptionType(i));
    networkInfo["rssi"] = WiFi.RSSI(i);
    networkInfo["bssid"] = WiFi.BSSIDstr(i);
    networkInfo["channel"] = WiFi.channel(i);
    networkInfo["hidden"] = WiFi.isHidden(i);
  }
  int jsonLength = root.measureLength() + 1;
  char* buffer = new char[jsonLength];
  root.printTo(buffer, jsonLength);
  return buffer;
}

String encryptionTypeStr(uint8_t authmode) {
  switch(authmode) {
    case ENC_TYPE_NONE:
        return "Open network";
    case ENC_TYPE_WEP:
        return "WEP";
    case ENC_TYPE_TKIP:
        return "WPA/PSK";
    case ENC_TYPE_CCMP:
        return "WPA2/PSK";
    case ENC_TYPE_AUTO:
        return "WPA/WPA2/PSK";
    default:
        return "?";
  }
}

void startServer() {
  detachInterrupt(BUTTON_PIN);
  WiFiAdapter.changeMode(SERVER);
  attachInterrupt(BUTTON_PIN, startClient, RISING);
}

void startClient() {
  detachInterrupt(BUTTON_PIN);
  WiFiAdapter.changeMode(CLIENT);
  attachInterrupt(BUTTON_PIN, startServer, RISING);
}

void loop() {
  WiFiAdapter.loop();
}

boolean IdleRunnableClass::run() {
  return true;
}

