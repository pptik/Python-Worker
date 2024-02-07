#ifndef CONFIG_H
#define CONFIG_H

#include <ArduinoJson.h>
#include <WiFi.h>
#include <Preferences.h>
#include "BluetoothSerial.h"
#include <PubSubClient.h>
#include "MFRC522.h"
#include "secret.h"
#define RESET_BUTTON 32
#define RST_PIN 22
#define SDA_PIN 21
#define BUZZER 26
#define LED_INDICATOR 12
int mqtt_port = 1883;
String clientIdWithPrefix = "ABSENSI-RFID";
char mqtt_server[40] = RMQ_SERVER;
char mqtt_user[40] = RMQ_USER;
char mqtt_password[40] = RMQ_PASSWORD;
char defaultpublish[40] = RMQ_PUBLISH;
char subscribe_mqtt[40];
char smqtt_port[5] = "1883";
char publish_mqtt[50];
char bluetooth_name[30];
const char* new_publish = "";
const char* pref_ssid = "";
const char* pref_pass = "";
String ssids_array[50];
String network_string;
String client_wifi_ssid;
String client_wifi_password;
long start_wifi_millis;
long wifi_timeout = 10000;
bool bluetooth_disconnect = false;
bool isButtonPressed = false;
unsigned long buttonPressTime;
const long interval = 2000;
unsigned long previousMillis = 0;
bool delayMeas = false;
int value = 0;

MFRC522 mfrc522(SDA_PIN, RST_PIN);
WiFiClient espClient;
BluetoothSerial SerialBT;
Preferences preferences;
PubSubClient client(espClient);

#endif