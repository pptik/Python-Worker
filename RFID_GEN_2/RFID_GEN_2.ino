#include "config.h"
#include "setup_wifiBT.h"
#include "setup_topic_publish.h"
#include "mac_address.h"
#include "rmq.h"

const char* guid = "35857778-4ef4-4b45-9e18-3c6079b74981";
String client_id = "ABSENSI-RFID-001";

void setup() {
  pinMode(BUZZER, OUTPUT);
  pinMode(LED_INDICATOR, OUTPUT);
  pinMode(RESET_BUTTON, INPUT_PULLUP);
  digitalWrite(BUZZER, LOW);
  digitalWrite(LED_INDICATOR, LOW);
  Serial.begin(115200);
  strcpy(subscribe_mqtt, guid);
  strcpy(bluetooth_name, "RFID-BT-");
  strcat(bluetooth_name, client_id.c_str());
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  setup_topic();
  client.setCallback(callbackrmq);
  delay(100);
  SPI.begin();
  mfrc522.PCD_Init();
}


void loop() {
  if (WiFi.status() != WL_CONNECTED) setup_wifi();
  if (bluetooth_disconnect) {
    disconnect_bluetooth();
  }
  digitalWrite(BUZZER, LOW);
  unsigned long currentMillis = millis();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  int buttonState = digitalRead(RESET_BUTTON);
  if (buttonState == LOW) {
    if (!isButtonPressed) {
      isButtonPressed = true;
      buttonPressTime = millis();
    } else {
      if (millis() - buttonPressTime >= 4000) {
        preferences.begin("my-app", false);
        preferences.clear();
        preferences.end();
        ESP.restart();
      }
    }
  } else {
    isButtonPressed = false;
  }

  if (delayMeas) {
    if ((currentMillis - previousMillis) >= interval) {
      previousMillis = currentMillis;
      delayMeas = false;
      delay(1800);
    }
  } else {
    if (!mfrc522.PICC_IsNewCardPresent()) {
      digitalWrite(BUZZER, LOW);
      delay(550);
      return;
    }

    if (!mfrc522.PICC_ReadCardSerial()) {
      digitalWrite(BUZZER, LOW);
      delay(900);
      return;
    }
    char msg[150];
    char msg1[150];
    msg[0] = char(0);
    msg1[0] = char(0);
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      sprintf(msg, "%s%02X", msg, mfrc522.uid.uidByte[i]);
    }
    StaticJsonDocument<200> doc;
    doc["PROJECT"] = clientIdWithPrefix;
    doc["GUID"] = guid;
    doc["MAC"] = getMACAddress();
    doc["DATA"] = msg;
    char out[150];
    serializeJson(doc, Serial);
    Serial.println("\t");
    int b = serializeJson(doc, out);
    if ((client.publish(publish_mqtt, out) == true)) {
      digitalWrite(BUZZER, HIGH);
      delay(1000);
    }
    msg[0] = char(0);
    delayMeas = true;
    digitalWrite(BUZZER, LOW);
    digitalWrite(LED_INDICATOR, HIGH);
    delay(220);
    digitalWrite(LED_INDICATOR, LOW);
    delay(100);
    digitalWrite(LED_INDICATOR, HIGH);
    delay(200);
    digitalWrite(LED_INDICATOR, LOW);
    delay(100);
    digitalWrite(LED_INDICATOR, HIGH);
    delay(100);
    digitalWrite(LED_INDICATOR, LOW);
    delay(100);
    digitalWrite(LED_INDICATOR, HIGH);
  }
}

