#ifndef mac_h
#define mac_h

#include "config.h"

String getMACAddress() {
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_BT);
  String macAddress = "";
  for (int i = 0; i < 6; ++i) {
    macAddress += String(mac[i], HEX);
    if (i < 5) {
      macAddress += ':';
    }
  }
  for (char& c : macAddress) {
    c = toupper(c);
  }
  return macAddress;
}

void printMACAddress() {
  String MAC = getMACAddress();

  Serial.print("BT MAC: ");
  Serial.println(MAC);
  SerialBT.print("BT MAC: ");
  SerialBT.println(MAC);
}

#endif