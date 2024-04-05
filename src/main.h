#include <Arduino.h>
#include <esp_now.h>
#include <ArduinoJson.h>

#define BOARD_NUM 1;
#define HEARTBEAT_TYPE 1;
#define FUNCTIONAL_TYPE 2;
#define MANAGER_TYPE 3;

JsonDocument json;
uint8_t masterAddress[] = {0x24, 0x6F, 0x28, 0x79, 0xD7, 0xC4};
String myMac;

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  char json[200];
} struct_message;

// Create a struct_message called message
struct_message message;

esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);

esp_err_t sendHeartBeat(){
  json["macOrigin"] = myMac;
  json["origin"] = BOARD_NUM;
  json["timeMicros"] = micros();
  json["coreTemp"] = temperatureRead();
  json["type"] = HEARTBEAT_TYPE;
  serializeJson(json, message.json, sizeof(message.json));
  esp_err_t result = esp_now_send(masterAddress, (uint8_t *) &message, sizeof(message));

  return result;
}