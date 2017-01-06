/*
  StoredCredentials.h - library which makes use of EEPROM in order to store single credentials pair there.
  Created by Dmytro Firago, September 27, 2016.
*/

#ifndef StoredCredentials_h
#define StoredCredentials_h

#define EEPROM_SSID_SIZE 32
#define EEPROM_PASSWORD_SIZE 64

#include "Arduino.h"

struct Credentials {
  char ssid[EEPROM_SSID_SIZE];
  char password[EEPROM_PASSWORD_SIZE];
};

class StoredCredentialsClass {

  private:
    Credentials _credentials;
	void initialize();
  
  public:
	StoredCredentialsClass();
    void create(String ssid, String password);
	char* getSsid();
	char* getPassword();
};

static StoredCredentialsClass StoredCredentials;
  
#endif