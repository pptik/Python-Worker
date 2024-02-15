#ifndef rmq_h
#define rmq_h

#include "config.h"

void callbackrmq(char* topic, byte* payload, unsigned int length) {
  delay(10);
  preferences.begin("my-app", false);
  Serial.print("Message arrived : ");
  payload[length] = '\0';
  String payloadStr = String((char*)payload);
  DynamicJsonDocument doc(200);
  DeserializationError error = deserializeJson(doc, payloadStr);
  if (error) {
    Serial.print(F("Failed to parse JSON: "));
    Serial.println(error.c_str());
    return;
  }
  const char* commandType = doc["command"];
  const char* route = doc["route"];
  if (strcmp(commandType, "ChangeRoute") == 0) {
    strcpy(publish_mqtt, route);
    Serial.println(publish_mqtt);
    Serial.println("Publish topic changed to new-publish-topic");
    preferences.putString("new_publish", route);
  }
  if (strcmp(commandType, "DefaultRoute") == 0) {
    strcpy(publish_mqtt, defaultpublish);
    Serial.println(publish_mqtt);
    Serial.println("Publish topic changed to new-publish-topic");
    preferences.putString("new_publish", defaultpublish);
  }
  if (strcmp(commandType, "RESET") == 0) {
    preferences.clear();
    preferences.end();
    ESP.restart();
  }
  preferences.end();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    Serial.println(mqtt_server);
    if (client.connect(clientIdWithPrefix.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("connected");
      digitalWrite(BUZZER, HIGH);
      delay(500);
      digitalWrite(BUZZER, LOW);
      digitalWrite(LED_INDICATOR, HIGH);
      delay(500);
      digitalWrite(LED_INDICATOR, LOW);
      delay(500);
      digitalWrite(LED_INDICATOR, HIGH);
      client.subscribe(subscribe_mqtt);
    } else {
      digitalWrite(LED_INDICATOR, HIGH);
      delay(200);
      digitalWrite(LED_INDICATOR, LOW);
      delay(200);
      digitalWrite(LED_INDICATOR, HIGH);
      delay(200);
      digitalWrite(LED_INDICATOR, LOW);
      delay(200);
      digitalWrite(LED_INDICATOR, HIGH);
      delay(200);
      digitalWrite(LED_INDICATOR, LOW);
      delay(1000);
      Serial.print("failed, rc=");
      Serial.print(client.state());
      if (client.state() == 4) ESP.restart();
      else {
        Serial.println(" try again in 5 seconds");
        delay(5000);
      }
    }
  }
}

#endif