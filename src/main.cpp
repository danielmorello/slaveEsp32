#include <WiFi.h>
#include  "main.h"

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("Delivery Success");
    digitalWrite(LED_BUILTIN, HIGH);
    delay(5);
    digitalWrite(LED_BUILTIN, LOW);
  } else
  {
    Serial.println("Delivery Fail");
  }
}

void IRAM_ATTR startCountTime() {
  portENTER_CRITICAL(&synch);
  unsigned long now = micros();
  if ((micros() - lastDebounce) > debounceTime) {
    if (started == false) {
      json["macOrigin"] = myMac;
      json["origin"] = BOARD_NUM;
      json["timeMicros"] = now;
      json["coreTemp"] = temperatureRead();
      json["type"] = START_COUNT_TYPE;
      json["experiment"] = experiment;
      json["position"] = POSITION;
      serializeJson(json, message.json, sizeof(message.json));
      esp_err_t result = esp_now_send(masterAddress, (uint8_t *) &message, sizeof(message));
      started = !started;
    }else {
      json["macOrigin"] = myMac;
      json["origin"] = BOARD_NUM;
      json["timeMicros"] = now;
      json["coreTemp"] = temperatureRead();
      json["type"] = END_COUNT_TYPE;
      json["experiment"] = experiment;
      json["position"] = POSITION;
      serializeJson(json, message.json, sizeof(message.json));
      esp_err_t result = esp_now_send(masterAddress, (uint8_t *) &message, sizeof(message));
      started = !started;
      experiment++;
    }
    lastDebounce = micros();
  }
  portEXIT_CRITICAL(&synch);
}

void setup() {
  Serial.begin(115200);
 
  WiFi.mode(WIFI_STA);
  myMac = WiFi.macAddress();

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);
  
  memcpy(peerInfo.peer_addr, masterAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(START_PIN, INPUT_PULLDOWN);
  attachInterrupt(START_PIN, startCountTime, RISING);
}
 
void loop() {   
  esp_err_t result = sendHeartBeat();
  delay(60000);
}