#include <Arduino.h>
#include <esp_now.h>
#include <ArduinoJson.h>

#define BOARD_NUM 5
#define POSITION OUTSIDE

#define INSIDE 1
#define OUTSIDE 0
#define HEARTBEAT_TYPE 1
#define START_COUNT_TYPE 2
#define END_COUNT_TYPE 3
#define MANAGER_TYPE 4
#define START_PIN 18
#define ON_BOARD_LED 2

const unsigned long debounceTime = 500000;
volatile unsigned long lastDebounce = 0;
volatile bool started = false;

portMUX_TYPE synch = portMUX_INITIALIZER_UNLOCKED;

JsonDocument json;
uint8_t masterAddress[] = {0x24, 0x6F, 0x28, 0x79, 0xD7, 0xC4};
String myMac;
unsigned long experiment = 1; 

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  char json[200];
} struct_message;

// Create a struct_message called message
struct_message message;

esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);

esp_err_t sendHeartBeat() {
  json["macOrigin"] = myMac;
  json["origin"] = BOARD_NUM;
  json["timeMicros"] = micros();
  json["coreTemp"] = temperatureRead();
  json["type"] = HEARTBEAT_TYPE;
  json["experiment"] = experiment;
  json["position"] = POSITION;
  serializeJson(json, message.json, sizeof(message.json));
  esp_err_t result = esp_now_send(masterAddress, (uint8_t *) &message, sizeof(message));

  return result;
}