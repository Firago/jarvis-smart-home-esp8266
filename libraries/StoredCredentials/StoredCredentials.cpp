#include "EEPROM.h"
#include "StoredCredentials.h"

const int EEPROM_SIZE = EEPROM_SSID_SIZE + EEPROM_PASSWORD_SIZE;
const int EEPROM_CREDENTIALS_OFFSET = 0;

StoredCredentialsClass::StoredCredentialsClass() {
  initialize();
}

void StoredCredentialsClass::initialize() {
  EEPROM.begin(EEPROM_SIZE);
};

void StoredCredentialsClass::create(String ssid, String password) {
  ssid.toCharArray(_credentials.ssid, EEPROM_SSID_SIZE);
  password.toCharArray(_credentials.password, EEPROM_PASSWORD_SIZE);
  EEPROM.put(EEPROM_CREDENTIALS_OFFSET, _credentials);
  EEPROM.commit();
};

char* StoredCredentialsClass::getSsid() {
  EEPROM.get(EEPROM_CREDENTIALS_OFFSET, _credentials);
  return _credentials.ssid;
};

char* StoredCredentialsClass::getPassword() {
  EEPROM.get(EEPROM_CREDENTIALS_OFFSET, _credentials);
  return _credentials.password;
};