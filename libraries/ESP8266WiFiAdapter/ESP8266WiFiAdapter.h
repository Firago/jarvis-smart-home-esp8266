/*
  ESP8266_WiFiAdapter.h - Library for ESP8266 which allows to manage WiFi mode. It may be set to one of the following states:
  1) IDLE - by default module in this mode does noting
  2) Server - enables WiFi hotspot with specified credentials
  3) Client - connects to the selected 
  Created by Dmytro Firago, September 22, 2016.
*/

#ifndef ESP8266WiFiAdapter_h
#define ESP8266WiFiAdapter_h

#include "Arduino.h"
#include "ESP8266WebServer.h"
#include "CountdownTimer.h"
#include "PubSubClient.h"

#define CLIENT_CONNECTION_TIMEOUT 10000
#define CHANGE_MODE_RETRY_TIMEOUT 1000
#define WIFI_STATUS_CHECK_FREQUENCY 50
#define SERVER_PORT 80

enum AdapterMode {
  IDLE 		= 0,
  SERVER 	= 1,
  CLIENT 	= 2
};

class DeviceRunnableClass {
  public:
    virtual boolean run() 	= 0;
    virtual boolean start() = 0;
    virtual boolean stop() 	= 0;
};

class IdleRunnableClass : public DeviceRunnableClass {
  public:
    boolean run();
    boolean start();
    boolean stop();
};

class ClientRunnableClass : public DeviceRunnableClass {
  public:
	ClientRunnableClass();
    boolean run();
    boolean start();
    boolean stop();
  private:
	WiFiClient _wifiClient;
	PubSubClient _mqttClient;
	boolean isWifiConnected();
	boolean isMqttConnected();
	boolean wifiConnect();
	boolean mqttConnect();
	boolean mqttSubscribe();
	boolean connectAndSubscribe();
};

class ServerRunnableClass : public DeviceRunnableClass {
  public:
    boolean run();
    boolean start();
    boolean stop();
};

class HttpRequestHandlerClass {
  public:
	void on(const char* uri, ESP8266WebServer::THandlerFunction handler);
	void on(const char* uri, HTTPMethod method, ESP8266WebServer::THandlerFunction fn);
	void send(int code, const char* content_type = NULL, const String& content = String(""));
	void send(int code, char* content_type, const String& content);
	void send(int code, const String& content_type, const String& content);
	int args();
	String arg(int i);
	String arg(String name);
};

class DeviceRunnableFactory {
  public:
	static DeviceRunnableClass* getInstance(int adapterMode);
};

class WiFiAdapterClass {
  public:
	WiFiAdapterClass();
    void changeMode(int adapterMode) 				volatile;
    void changeMode(int adapterMode, long period) 	volatile;
    void loop() 									volatile;
  private:
    DeviceRunnableClass* 	_deviceRunnable;
	Timer* 					_switchBackTimer;
	int 					_previousMode;
	int 					_currentMode;
	boolean 				_temporaryMode;
	void initialize();
    void switchMode(int adapterMode) 				volatile;
};

static WiFiAdapterClass WiFiAdapter;

static IdleRunnableClass IdleRunnable;
static ServerRunnableClass ServerRunnable;
static ClientRunnableClass ClientRunnable;

static ESP8266WebServer Server = ESP8266WebServer(SERVER_PORT);

static HttpRequestHandlerClass HttpRequestHandler;

#endif
