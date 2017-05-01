#include <ESP8266WiFiAdapter.h>
#include <StoredCredentials.h>
#include <ArduinoJson.h>

// extern initialization section
char*   WIFI_AP_SSID        = "JSH_SWITCH1";
char*   WIFI_AP_PASSWORD    = "";

char*   MQTT_SERVER         = "172.24.1.1";
int     MQTT_PORT           = 1883;
char*   MQTT_USER           = "";
char*   MQTT_PASSWORD       = "";
char*   MQTT_MODULE_ID      = "24D6EE34E1CC48ED9F302C72A946222A";
char*   MQTT_CLIENT_TOPIC   = "consumers";

int     IDLE_PIN            = 16;
int     CLIENT_PIN          = 4;
int     SERVER_PIN          = 5;
int     BUTTON_PIN          = 13;
int     SWITCH_PIN          = 14;

const char* CONTENT_TYPE_JSON = "application/json";

void mqttClientCallback(char* topic, byte* payload, unsigned int length) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& request = jsonBuffer.parseObject((char*) payload);
  String deviceId = request["deviceId"];
  if (deviceId == MQTT_MODULE_ID) {
    String action = request["action"];
    if (action == "turn on") {
      digitalWrite(SWITCH_PIN, HIGH);
    } else if (action == "turn off") {
      digitalWrite(SWITCH_PIN, LOW);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(SWITCH_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  setAdapterMode();
  setupRequestHandler();
}

void setAdapterMode() {
  if (digitalRead(BUTTON_PIN) == HIGH) {
    WiFiAdapter.changeMode(SERVER);
  } else {
    WiFiAdapter.changeMode(CLIENT);
  }
}

void setupRequestHandler() {
  HttpRequestHandler.on("/ping", HTTP_GET, [](){
    HttpRequestHandler.send(200, CONTENT_TYPE_JSON, "{\"success\": \"true\"}");
  });
  HttpRequestHandler.on("/module/info", HTTP_GET, [](){
    char* body = getDeviceInfo();
    HttpRequestHandler.send(200, CONTENT_TYPE_JSON, body);
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
    HttpRequestHandler.send(200, CONTENT_TYPE_JSON, "{\"success\": \"true\"}");
  });
}

char* getDeviceInfo() {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["moduleId"] = MQTT_MODULE_ID;
  int jsonLength = root.measureLength() + 1;
  char* buffer = new char[jsonLength];
  root.printTo(buffer, jsonLength);
  return buffer;
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

void loop() {
  WiFiAdapter.loop();
}

