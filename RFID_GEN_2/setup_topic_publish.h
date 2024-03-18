#ifndef topic_publish
#define topic_publish

#include "config.h"

void setup_topic() {
  preferences.begin("my-app", false);
  String new_p = preferences.getString("new_publish");
  new_publish = new_p.c_str();
  if (new_p == "") {
    Serial.println("No values saved for new topic");
    Serial.println("Back to default");
    strcpy(publish_mqtt, defaultpublish);
  } else {
    Serial.println("Connect to last topic");
    strcpy(publish_mqtt, new_publish);
  }
  preferences.end();
}

#endif