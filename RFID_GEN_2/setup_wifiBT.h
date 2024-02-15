#ifndef setup_wifiBT
#define setup_wifiBT

#include "config.h"
#include "mac_address.h"

void scan_wifi_networks() {
  DynamicJsonDocument nonetwork(20);
  nonetwork["data"] = "no network found";
  String no_network;
  serializeJson(nonetwork, no_network);
  DynamicJsonDocument network(20);
  network["data"] = "network found";
  String networkfnd;
  serializeJson(network, networkfnd);
  WiFi.mode(WIFI_STA);
  int n = WiFi.scanNetworks();
  if (n == 0) {
    // SerialBT.println(no_network);
  } else {
    // SerialBT.println();
    Serial.println(networkfnd);
    delay(1000);
    DynamicJsonDocument scanwifi(256);
    String jsonscanWifi;
    JsonArray scanWifiArray = scanwifi.createNestedArray("data");
    for (int i = 0; i < n; ++i) {
      ssids_array[i + 1] = WiFi.SSID(i);
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.println(ssids_array[i + 1]);
      network_string = i + 1;
      network_string = network_string + ": " + WiFi.SSID(i) + " (Strength:" + WiFi.RSSI(i) + ")";
      scanWifiArray.add(WiFi.SSID(i));
    }

    serializeJson(scanwifi, jsonscanWifi);
    SerialBT.print(jsonscanWifi);
    Serial.println("Please enter the number for your Wi-Fi");
  }
}

void callback(esp_spp_cb_event_t event, esp_spp_cb_param_t* param) {
  if (event == ESP_SPP_SRV_OPEN_EVT) {
    Serial.println("Scanning Wi-Fi networks");
    scan_wifi_networks();
  }
}

void disconnect_bluetooth() {
  delay(1000);
  DynamicJsonDocument disconnect(20);
  disconnect["data"] = "Bluetooth disconnecting...";
  String bt_disconnect;
  serializeJson(disconnect, bt_disconnect);
  Serial.println("BT stopping");
  SerialBT.print(bt_disconnect);
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

    DynamicJsonDocument selectedWiFi(20);
    selectedWiFi["data"] = "Selected Wifi";
    String bt_selectedWiFi;
    serializeJson(selectedWiFi, bt_selectedWiFi);
    SerialBT.print(bt_selectedWiFi);

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
    Serial.println("Please wait for Wi_Fi connection...");
    WiFi.begin(client_wifi_ssid.c_str(), client_wifi_password.c_str());
    start_wifi_millis = millis();
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      if (millis() - start_wifi_millis > wifi_timeout) {
        WiFi.disconnect(true, true);
        DynamicJsonDocument cantConnect(20);
        cantConnect["data"] = "Cant Connect";
        String bt_cantconnect;
        serializeJson(cantConnect, bt_cantconnect);
        Serial.println("Cant Connect");
        SerialBT.print(bt_cantconnect);

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
    DynamicJsonDocument connected(20);
    connected["data"] = "Connected";
    String wifi_connected;
    serializeJson(connected, wifi_connected);
    SerialBT.print(wifi_connected);
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
      DynamicJsonDocument ip3(50);
      ip3["data"] = WiFi.localIP().toString();
      String ipaddress;
      serializeJson(ip3, ipaddress);
      SerialBT.println(ipaddress);
      preferences.putString("pref_ssid", client_wifi_ssid);
      preferences.putString("pref_pass", client_wifi_password);
      bluetooth_disconnect = true;
      delay(500);
      DynamicJsonDocument act(20);
      act["data"] = "Device Activated";
      String activated;
      serializeJson(act, activated);
      SerialBT.println(activated);
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
        Serial.println("Couldn't connect to Wi-Fi...");
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
    DynamicJsonDocument connected1(20);
    connected1["data"] = "Connected";
    String wifi_connected1;
    serializeJson(connected1, wifi_connected1);
    SerialBT.print(wifi_connected1);
    Serial.println("Connected");
    digitalWrite(LED_INDICATOR, HIGH);
    printMACAddress();
    Serial.print("ESP32 IP: ");
    Serial.println(WiFi.localIP());
    bluetooth_disconnect = true;
  }
  preferences.end();
}

#endif
