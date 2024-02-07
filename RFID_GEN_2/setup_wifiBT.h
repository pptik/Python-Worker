#ifndef setup_wifiBT
#define setup_wifiBT

#include "config.h"
#include "mac_address.h"

void scan_wifi_networks() {
  WiFi.mode(WIFI_STA);
  int n = WiFi.scanNetworks();
  if (n == 0) {
    SerialBT.println("no networks found");
  } else {
    SerialBT.println();
    SerialBT.print(n);
    SerialBT.println(" networks found");
    delay(1000);
    for (int i = 0; i < n; ++i) {
      ssids_array[i + 1] = WiFi.SSID(i);
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.println(ssids_array[i + 1]);
      network_string = i + 1;
      network_string = network_string + ": " + WiFi.SSID(i) + " (Strength:" + WiFi.RSSI(i) + ")";
      SerialBT.println(network_string);
    }
    SerialBT.println("Please enter the number for your Wi-Fi");
    Serial.println("Please enter the number for your Wi-Fi");
  }
}

void callback(esp_spp_cb_event_t event, esp_spp_cb_param_t* param) {
  if (event == ESP_SPP_SRV_OPEN_EVT) {
    SerialBT.println("Scanning Wi-Fi networks");
    Serial.println("Scanning Wi-Fi networks");
    scan_wifi_networks();
  }
}

void callback_show_ip(esp_spp_cb_event_t event, esp_spp_cb_param_t* param) {
  if (event == ESP_SPP_SRV_OPEN_EVT) {
    SerialBT.print("ESP32 IP: ");
    SerialBT.println(WiFi.localIP());
    bluetooth_disconnect = true;
  }
}

void disconnect_bluetooth() {
  delay(1000);
  Serial.println("BT stopping");
  SerialBT.println("Bluetooth disconnecting...");
  delay(1000);
  SerialBT.flush();
  SerialBT.disconnect();
  SerialBT.end();
  Serial.println("BT stopped");
  delay(1000);
  bluetooth_disconnect = false;
}

void setup_wifi() {
  delay(10);
  preferences.begin("my-app", false);
  String ssid = preferences.getString("pref_ssid");
  String pass = preferences.getString("pref_pass");
  pref_ssid = ssid.c_str();
  pref_pass = pass.c_str();
  Serial.println(pref_ssid);
  Serial.println(pref_pass);
  if (ssid == "" && pass == "") {
    Serial.println("No values saved for ssid or password");
    SerialBT.begin(bluetooth_name);
    Serial.printf("Device name: \"%s\"\n", bluetooth_name);
    SerialBT.register_callback(callback);
A:
    while (!(SerialBT.available())) {
      digitalWrite(LED_INDICATOR, HIGH);
      delay(1000);
      digitalWrite(LED_INDICATOR, LOW);
      delay(2000);
    }
    while (SerialBT.available()) {
      int client_wifi_ssid_id = SerialBT.readString().toInt();
      client_wifi_ssid = ssids_array[client_wifi_ssid_id];
    }
    SerialBT.println("Please enter your Wi-Fi password");
    Serial.println("Please enter your Wi-Fi password");
    while (!(SerialBT.available())) {
      digitalWrite(LED_INDICATOR, HIGH);
      delay(1000);
      digitalWrite(LED_INDICATOR, LOW);
      delay(2000);
    }
    while (SerialBT.available()) {
      client_wifi_password = SerialBT.readString();
      client_wifi_password.trim();
    }
    delay(500);
    SerialBT.println("Please wait for Wi-Fi connection...");
    Serial.println("Please wait for Wi_Fi connection...");
    WiFi.begin(client_wifi_ssid.c_str(), client_wifi_password.c_str());
    start_wifi_millis = millis();
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      if (millis() - start_wifi_millis > wifi_timeout) {
        WiFi.disconnect(true, true);
        Serial.println("Could't connect to Wi-Fi...");
        SerialBT.println("Could't connect to Wi-Fi...");
        digitalWrite(LED_INDICATOR, HIGH);
        delay(200);
        digitalWrite(LED_INDICATOR, LOW);
        delay(200);
        digitalWrite(LED_INDICATOR, HIGH);
        delay(200);
        digitalWrite(LED_INDICATOR, LOW);
        delay(200);
        scan_wifi_networks();
        goto A;
      }
    }
    SerialBT.println("Connected");
    Serial.println("Connected");
    while (!(SerialBT.available())) {
      digitalWrite(LED_INDICATOR, HIGH);
      delay(1000);
      digitalWrite(LED_INDICATOR, LOW);
      delay(2000);
    }
    int mobileActivated = SerialBT.readString().toInt();
    if (mobileActivated == 1) {
      digitalWrite(LED_INDICATOR, HIGH);
      printMACAddress();
      Serial.print("ESP32 IP: ");
      Serial.println(WiFi.localIP());
      SerialBT.print("ESP32 IP: ");
      SerialBT.println(WiFi.localIP());
      preferences.putString("pref_ssid", client_wifi_ssid);
      preferences.putString("pref_pass", client_wifi_password);
      bluetooth_disconnect = true;
      SerialBT.println("Device Activated");
      Serial.println("Device Activated");
    } else {
      Serial.print("mobileActivated : ");
      Serial.println("false");
      ESP.restart();
    }

  } else {
    WiFi.begin(pref_ssid, pref_pass);
    start_wifi_millis = millis();
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      digitalWrite(LED_INDICATOR, HIGH);
      delay(200);
      digitalWrite(LED_INDICATOR, LOW);
      delay(200);
      digitalWrite(LED_INDICATOR, HIGH);
      delay(200);
      digitalWrite(LED_INDICATOR, LOW);
      delay(1000);
      Serial.print(".");
      if (millis() - start_wifi_millis > wifi_timeout) {
        WiFi.disconnect(true, true);
        Serial.println("Could't connect to Wi-Fi...");
        digitalWrite(LED_INDICATOR, HIGH);
        delay(200);
        digitalWrite(LED_INDICATOR, LOW);
        delay(200);
        digitalWrite(LED_INDICATOR, HIGH);
        delay(200);
        digitalWrite(LED_INDICATOR, LOW);
        delay(1000);
        setup_wifi();
      }
    }
    SerialBT.println("Connected");
    Serial.println("Connected");
    digitalWrite(LED_INDICATOR, HIGH);
    printMACAddress();
    Serial.print("ESP32 IP: ");
    Serial.println(WiFi.localIP());
    SerialBT.print("ESP32 IP: ");
    SerialBT.println(WiFi.localIP());
    bluetooth_disconnect = true;
  }
  preferences.end();
}

#endif