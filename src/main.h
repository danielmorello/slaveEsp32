#include <Arduino.h>
#include <esp_now.h>
#include <ArduinoJson.h>

#define BOARD_NUM 4
#define POSITION OUTSIDE

#define INSIDE 1
#define OUTSIDE 0
#define HEARTBEAT_TYPE 1
#define START_COUNT_TYPE 2
#define END_COUNT_TYPE 3
#define MANAGER_TYPE 4
#define START_PIN 18
#define ON_BOARD_LED 2
#define HEARTBEAT_INTERVAL 60000
#define RESTART_TIME 2000

portMUX_TYPE synch = portMUX_INITIALIZER_UNLOCKED;

const unsigned long debounceTime = 500000;
volatile unsigned long lastDebounce = 0;
volatile bool started = false;

uint8_t masterAddress[6] = { 0x24, 0x6F, 0x28, 0x79, 0xD7, 0xC4 };
esp_now_peer_info_t peerInfo;

unsigned long experiment = 1; 

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
  message.position = POSITION;
  esp_err_t result = esp_now_send(masterAddress, (uint8_t *) &message, sizeof(message));

  return result;
}