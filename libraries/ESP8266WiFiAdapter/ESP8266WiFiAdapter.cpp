#include "ESP8266WiFiAdapter.h"
#include "StoredCredentials.h"
#include "ESP8266WebServer.h"

// extern definition section
extern char*	WIFI_AP_SSID;
extern char* 	WIFI_AP_PASSWORD;

extern char*   	MQTT_SERVER;
extern int     	MQTT_PORT;
extern char*   	MQTT_USER;
extern char*   	MQTT_PASSWORD;
extern char* 	MQTT_MODULE_ID;
extern char*	MQTT_CLIENT_TOPIC;

extern int 		IDLE_PIN;
extern int 		CLIENT_PIN;
extern int 		SERVER_PIN;

extern void mqttClientCallback(char* topic, byte* payload, unsigned int length);

boolean IdleRunnableClass::start() {
  digitalWrite(IDLE_PIN, HIGH);
  return true;
}

boolean IdleRunnableClass::stop() {
  digitalWrite(IDLE_PIN, LOW);
  return true;
}

ClientRunnableClass::ClientRunnableClass() {
  _wifiClient = WiFiClient();
  _mqttClient = PubSubClient();
  _mqttClient.setClient(_wifiClient);
}

boolean ClientRunnableClass::start() {
  digitalWrite(CLIENT_PIN, HIGH);
  WiFi.mode(WIFI_STA);
  return connectAndSubscribe();
}

boolean ClientRunnableClass::run() {
  boolean isConnected = (isWifiConnected() && isMqttConnected()) || connectAndSubscribe();
  if (isConnected) {
	_mqttClient.loop();
  }
  return isConnected;
}

boolean ClientRunnableClass::stop() {
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  digitalWrite(CLIENT_PIN, LOW);
  return true;
}

boolean ClientRunnableClass::isWifiConnected() {
  return WiFi.status() == WL_CONNECTED;
}

boolean ClientRunnableClass::isMqttConnected() {
  return _mqttClient.connected();
}

boolean ClientRunnableClass::wifiConnect() {
  WiFi.begin(StoredCredentials.getSsid(), StoredCredentials.getPassword());
  Timer timer(CLIENT_CONNECTION_TIMEOUT);
  while (!timer.expired()
		&& WiFi.status() != WL_CONNECTED
		&& WiFi.status() != WL_NO_SSID_AVAIL 
		&& WiFi.status() != WL_CONNECT_FAILED) {
    delay(WIFI_STATUS_CHECK_FREQUENCY);
  }
  return isWifiConnected();
}

boolean ClientRunnableClass::mqttConnect() {
  _mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  _mqttClient.setCallback(mqttClientCallback);
  _mqttClient.connect(MQTT_MODULE_ID, MQTT_USER, MQTT_PASSWORD);
  return isMqttConnected();
}

boolean ClientRunnableClass::mqttSubscribe() {
  return _mqttClient.subscribe(MQTT_CLIENT_TOPIC);
}

boolean ClientRunnableClass::connectAndSubscribe() {
  boolean wifiConnected = isWifiConnected() || wifiConnect();
  boolean mqttConnected = wifiConnected && (isMqttConnected() || mqttConnect());
  boolean mqttSubscribed = mqttConnected && mqttSubscribe();
  return mqttSubscribed;
}

boolean ServerRunnableClass::start() {
  digitalWrite(SERVER_PIN, HIGH);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD);
  Server.begin();
  return true;
}

boolean ServerRunnableClass::run() {
  Server.handleClient();
  return true;
}

boolean ServerRunnableClass::stop() {
  WiFi.softAPdisconnect();
  WiFi.mode(WIFI_OFF);
  digitalWrite(SERVER_PIN, LOW);
  return true;
}

void HttpRequestHandlerClass::on(const char* uri, ESP8266WebServer::THandlerFunction handler) {
  Server.on(uri, handler);
}

void HttpRequestHandlerClass::on(const char* uri, HTTPMethod method, ESP8266WebServer::THandlerFunction fn) {
  Server.on(uri, method, fn);
}

void HttpRequestHandlerClass::send(int code, const char* content_type, const String& content) {
  Server.send(code, content_type, content);
}

void HttpRequestHandlerClass::send(int code, const String& content_type, const String& content) {
  Server.send(code, content_type, content);
}

int HttpRequestHandlerClass::args() {
  return Server.args();
}

String HttpRequestHandlerClass::arg(int i) {
  return Server.arg(i);
}

String HttpRequestHandlerClass::arg(String name) {
  return Server.arg(name);
}

DeviceRunnableClass* DeviceRunnableFactory::getInstance(int adapterMode) {
  switch (adapterMode) {
    case IDLE:
      return &IdleRunnable;
    case SERVER:
      return &ServerRunnable;
    case CLIENT:
      return &ClientRunnable;
  }
}

WiFiAdapterClass::WiFiAdapterClass() {
  _deviceRunnable = DeviceRunnableFactory::getInstance(IDLE);
  _currentMode = IDLE;
  _temporaryMode = false;
  initialize();
}

void WiFiAdapterClass::changeMode(int adapterMode) volatile {
  if (_currentMode != adapterMode) {
	_deviceRunnable->stop();
	_deviceRunnable = DeviceRunnableFactory::getInstance(adapterMode);
	switchMode(adapterMode);
	if (!_deviceRunnable->start()) {
	  changeMode(IDLE, CHANGE_MODE_RETRY_TIMEOUT);
	}
  }
}

void WiFiAdapterClass::changeMode(int adapterMode, long period) volatile {
  _switchBackTimer = Timer::start(period);
  _temporaryMode = true;
  changeMode(adapterMode);
}

void WiFiAdapterClass::loop() volatile {
  if (_temporaryMode && _switchBackTimer->expired()) {
	_temporaryMode = false;
	changeMode(_previousMode);
  } else {
	_deviceRunnable->run();
  }
}

void WiFiAdapterClass::initialize() {
  WiFi.mode(WIFI_OFF);
  pinMode(IDLE_PIN, OUTPUT);
  pinMode(CLIENT_PIN, OUTPUT);
  pinMode(SERVER_PIN, OUTPUT);
}

void WiFiAdapterClass::switchMode(int adapterMode) volatile {
  _previousMode = _currentMode;
  _currentMode = adapterMode;
}