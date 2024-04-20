#include <WiFi.h>
#include  "main.h"

void restartByMasterCommand() {
  delay(RESTART_TIME);
  esp_restart();
}

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

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&masterMessage, incomingData, sizeof(masterMessage));
  if (masterMessage.restart == true) {
    restartByMasterCommand();
  }
}

void IRAM_ATTR startCountTime() {
  portENTER_CRITICAL(&synch);
  unsigned long now = micros();
  if ((micros() - lastDebounce) > debounceTime) {
    if (started == false) {
      message.origin = BOARD_NUM;
      message.timeMicros = now;
      message.coreTemp = temperatureRead();
      message.type = START_COUNT_TYPE;
      message.experiment = experiment;
      esp_err_t result = esp_now_send(masterAddress, (uint8_t *) &message, sizeof(message));
      started = !started;
    }else {
      message.origin = BOARD_NUM;
      message.timeMicros = now;
      message.coreTemp = temperatureRead();
      message.type = END_COUNT_TYPE;
      message.experiment = experiment;
      esp_err_t result = esp_now_send(masterAddress, (uint8_t *) &message, sizeof(message));
      started = !started;
      experiment++;
    }
    lastDebounce = micros();
  }
  portEXIT_CRITICAL(&synch);
}

void setup() {
  pinMode(POSITION_PIN, INPUT);
  if (POSITION_PIN == HIGH) {
    message.position = INSIDE;
  } else if (POSITION_PIN == LOW) {
    message.position = OUTSIDE;
  } else {
    message.position = OUTSIDE;
  }

  Serial.begin(115200);
 
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.macAddress(message.macOrigin);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
  
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
  loopCount++;
  esp_err_t result = sendHeartBeat();
  delay(HEARTBEAT_INTERVAL);
}