#include <Arduino.h>
#include <esp_now.h>
#include <ArduinoJson.h>

#define BOARD_NUM 10

#define INSIDE 1
#define OUTSIDE 0
#define HEARTBEAT_TYPE 1
#define START_COUNT_TYPE 2
#define END_COUNT_TYPE 3
#define MANAGER_TYPE 4
#define START_PIN 18
#define POSITION_PIN 23
#define HEARTBEAT_INTERVAL 120000
#define RESTART_TIME 2000

portMUX_TYPE synch = portMUX_INITIALIZER_UNLOCKED;

const unsigned long debounceTime = 500000;
volatile unsigned long lastDebounce = 0;
volatile bool started = false;

uint8_t masterAddress[6] =  { 0x48, 0xE7, 0x29, 0xC9, 0x2B, 0x58 };  // 1
// uint8_t masterAddress[6] =  { 0xA8, 0x42, 0xE3, 0x59, 0x83, 0x38 };  // 2
// uint8_t masterAddress[6] =  { 0x48, 0xE7, 0x29, 0xC9, 0xED, 0x00 };  // 3
// uint8_t masterAddress[6] =  { 0x48, 0xE7, 0x29, 0xCA, 0x15, 0xD0 };  // 4
// uint8_t masterAddress[6] =  { 0xC8, 0xF0, 0x9E, 0xF5, 0x17, 0x94 };  // 5
// uint8_t masterAddress[6] =  { 0xC8, 0xF0, 0x9E, 0xF8, 0x72, 0x70 };  // 6
// uint8_t masterAddress[6] =  { 0xC8, 0xF0, 0x9E, 0xF7, 0x61, 0x98 };  // 7
// uint8_t masterAddress[6] =  { 0x7C, 0x9E, 0xBD, 0x3A, 0x01, 0x04 };  // 8
// uint8_t masterAddress[6] =  { 0xC8, 0xF0, 0x9E, 0xF8, 0x6A, 0x98 };  // 9
// uint8_t masterAddress[6] =  { 0xC8, 0xF0, 0x9E, 0xF7, 0x69, 0x98 }; // 10

esp_now_peer_info_t peerInfo;

unsigned long experiment = 1; 
unsigned long loopCount = 1; 

typedef struct Message {
  int origin;
  uint8_t macOrigin[6];
  unsigned long timeMicros;
  float coreTemp;
  int type;
  unsigned long experiment;
  int position;
} Message;

Message message;

typedef struct MasterMessage {
  bool restart;
} MasterMessage;

MasterMessage masterMessage;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len);

esp_err_t sendHeartBeat() {
  message.origin = BOARD_NUM;
  message.timeMicros = micros();
  message.coreTemp = temperatureRead();
  message.type = HEARTBEAT_TYPE;
  message.experiment = experiment;
  esp_err_t result = esp_now_send(masterAddress, (uint8_t *) &message, sizeof(message));

  return result;
}