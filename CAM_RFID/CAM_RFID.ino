#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <WiFiManager.h>
#include "esp_camera.h"
#include <ESP32_FTPClient.h>
#include <ArduinoJson.h>
#include "secret.h"
#include <Preferences.h>

#define RXD2 14
#define TXD2 15
#define RESET_BUTTON 2
#include <esp_task_wdt.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#define WDT_TIMEOUT_MS 60000
const int ledPin = 13;
const int buzzerPin = 12; 

TaskHandle_t Task1;
TaskHandle_t Task2;

volatile bool rfidDetected = false;
volatile bool streamingEnabled = false;

const char* mqtt_server = RMQ_SERVER;
const int mqtt_port = 1883;

const char* mqtt_user = RMQ_USER;
const char* mqtt_pass = RMQ_PASSWORD;

const char* mqtt_topic = "Streaming_Kamera";
const char* mqtt_topic_filename = TOPIC_REPORT;

char ftp_server[] = FTP_SERVER;
char ftp_user[] = FTP_USER;
char ftp_pass[] = FTP_PASSWORD;

ESP32_FTPClient ftp (ftp_server,2121,ftp_user,ftp_pass, 5000, 2);

WiFiClient espClient;
PubSubClient client(espClient);
WiFiManager wifiManager;
Preferences preferences;

String client_id = "ABCAM-AB10"; 
unsigned long lastMillis = 0;
const char* TOPIC = client_id.c_str(); //for device subscribe, dynamic per device
const char* mqtt_topic_subscribe = TOPIC;
//const char* TOPIC_Report = TOPIC_REPORT; //dont change, this one for server consumer
String clientIdWithPrefix = "ESP32Client-" + client_id;
String APName = "ESP32CAM-AP" + client_id;
unsigned long buttonPressTime;
bool isButtonPressed = false; 

String generateRandomString() {
  String randomString = "";
  char chars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

  for (int i = 0; i < 9; i++) {
    uint8_t randomValue = esp_random() % strlen(chars); // Pick one random character
    randomString += chars[randomValue];
  }

  return randomString;
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload, length);
  if (doc.containsKey("streaming")) {
    streamingEnabled = doc["streaming"];
//    client.publish(mqtt_topic_filename, filename.c_str());
  }
  Serial.println(streamingEnabled);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(clientIdWithPrefix.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      client.subscribe(mqtt_topic_subscribe);
      Serial.println("Subscribed to MQTT topic: " + String(mqtt_topic_subscribe));
      } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

//Bagian Setup Wifi -Fizar

void setup_wifi() {
  delay(10);
  preferences.begin("my-app", false);

  String ssid = preferences.getString("ssid", ""); //second parameter is default value
  String password = preferences.getString("password", "");

  // Check if we have stored WiFi credentials
  if(ssid == "" || password == "") {
    Serial.println("No values saved for ssid or password");    
    // Uncomment and run it once, if you want to erase all the stored information
//     wifiManager.resetSettings();/
    // If it cannot connect in a certain time, it starts an access point with the specified name
    if(!wifiManager.autoConnect(APName.c_str())) {
      Serial.println("Failed to connect and hit timeout");
      // Reset and try again, or maybe put it to deep sleep
      ESP.restart();
      delay(5000);
    }

    // Save the WiFi credentials in the preferences
    preferences.putString("ssid", WiFi.SSID());
    preferences.putString("password", WiFi.psk());
  }
  else {
    WiFi.begin(ssid.c_str(), password.c_str());
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.println("Failed to connect with saved credentials");
      ESP.restart();
    }
  }
  preferences.end();
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  delay(10);
  pinMode(ledPin, OUTPUT);
  pinMode(RESET_BUTTON, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  setup_wifi();
  // Configure the camera
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = 5;
  config.pin_d1 = 18;
  config.pin_d2 = 19;
  config.pin_d3 = 21;
  config.pin_d4 = 36;
  config.pin_d5 = 39;
  config.pin_d6 = 34;
  config.pin_d7 = 35;
  config.pin_xclk = 0;
  config.pin_pclk = 22;
  config.pin_vsync = 25;
  config.pin_href = 23;
  config.pin_sscb_sda = 26;
  config.pin_sscb_scl = 27;
  config.pin_pwdn = 32;
  config.pin_reset = -1;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_SVGA; 
  config.jpeg_quality = 10;
  config.fb_count = 1;

  // Initialize the camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

//  WiFi.begin(ssid, password);
//
//  while (WiFi.status() != WL_CONNECTED) {
//    delay(500);
//    Serial.print(".");
//  }
  Serial.println("WiFi connected");

  xTaskCreatePinnedToCore(
                    Task1code,   
                    "Task1",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task1,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  
  delay(500); 

  // create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
                    Task2code,   /* Task function. */
                    "Task2",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task2,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */
    delay(500); 

  //  espClient.setCACert(NULL); // Remove this line if using an unsecured connection
  client.setServer(mqtt_server, 1883);
  client.setCallback(mqttCallback);
  esp_task_wdt_init(WDT_TIMEOUT_MS, true); // 10 seconds time limit before rebooting
  esp_task_wdt_add(NULL); // add the loop() task to the watchdog
  
  delay(1000);
}

void loop() {
  esp_task_wdt_reset();
  if (!client.connected()) {
    reconnect();
    digitalWrite(ledPin, LOW);
  } else {
    digitalWrite(ledPin, HIGH);
  }
  client.loop();
  int buttonState = digitalRead(RESET_BUTTON);
  if (buttonState == LOW) {
    // Button is pressed
    if (!isButtonPressed) {
      isButtonPressed = true;
      buttonPressTime = millis();
    } else {
      // The button is being held down
      if (millis() - buttonPressTime >= 5000) { 
        wifiManager.resetSettings();
        preferences.begin("my-app", false);
        preferences.clear();
        preferences.end();
        ESP.restart(); // Restart ESP32
      }
    }
  } else {
    isButtonPressed = false;
  }
}

void Task1code( void * pvParameters ){
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  while (1) {
    if (!streamingEnabled || !client.connected() || rfidDetected) {
      vTaskDelay(pdMS_TO_TICKS(1000));
      continue;
    }

    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Failed to capture image");
      return;
    }
  
    // Send the image in chunks
    size_t chunk_size = 256;
    size_t chunks = (fb->len + chunk_size - 1) / chunk_size;
    
    for (size_t i = 0; i < chunks; i++) {
      size_t offset = i * chunk_size;
      size_t this_chunk_size = chunk_size;
      
      if (offset + chunk_size > fb->len) {
        this_chunk_size = fb->len - offset;
      }
      
      client.beginPublish(mqtt_topic, this_chunk_size, false);
      client.write(fb->buf + offset, this_chunk_size);
      client.endPublish();
    }
    
    esp_camera_fb_return(fb);
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}


void Task2code( void * pvParameters ){
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());
  
  while (1) {
    String receivedString = "";
    while (Serial2.available()) {
      char receivedChar = Serial2.read(); // Read a character from Serial2
      receivedString += receivedChar; // Add the character to the receivedString
    }
    if (receivedString.length() > 0) {
      receivedString.trim(); // Remove any leading/trailing white spaces
      // Check if the receivedString contains "Card UID:"
      int uidIndex = receivedString.indexOf("Card UID:");
      if (uidIndex >= 0) {
        rfidDetected = true; 
        String randomString = generateRandomString();
        digitalWrite(buzzerPin, HIGH);       
        String uidString = receivedString.substring(uidIndex + 10); // 10 is the length of "Card UID: "
        uidString.trim(); 
        uidString.replace(" ", ""); 
//        Serial.print("UID: ");
//        Serial.println(uidString);       
        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb) {
          Serial.println("Failed to capture image");
          receivedString = "";
          return;
        }                 
        // Create unique filename
        String filename = client_id + "_" + randomString + "_" + uidString + ".jpg";          
        ftp.OpenConnection();    
        ftp.ChangeWorkDir("/RFIDCAM/");
        ftp.InitFile("Type I");
        ftp.NewFile(filename.c_str());  
        ftp.WriteData(fb->buf, fb->len);        
        ftp.CloseFile();
        ftp.CloseConnection();
        digitalWrite(buzzerPin, LOW);
        esp_camera_fb_return(fb);        
        rfidDetected = false;        
        
        // Send the filename to the MQTT topic
        client.publish(mqtt_topic_filename, filename.c_str());        
      }
      receivedString = "";
    }
    vTaskDelay(pdMS_TO_TICKS(200));
  }  
}
