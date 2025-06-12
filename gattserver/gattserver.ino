// This is a simple test of GATT server for SOLE F85
/*
    Video: https://www.youtube.com/watch?v=oCMOYS71NIU
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updated by chegewara

   Create a BLE server that, once we receive a connection, will send periodic notifications.
   The service advertises itself as: 4fafc201-1fb5-459e-8fcc-c5c9c331914b
   And has a characteristic of: beb5483e-36e1-4688-b7f5-ea07361b26a8

   The design of creating the BLE server is:
   1. Create a BLE Server
   2. Create a BLE Service
   3. Create a BLE Characteristic on the Service
   4. Create a BLE Descriptor on the characteristic
   5. Start the service.
   6. Start advertising.

   A connect hander associated with the server starts a background task that performs notification
   every couple of seconds.
*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "util.h"

void clear_cmd_resp(void);


BLEServer* pServer = NULL;
BLECharacteristic* pFmtsCharacteristic = NULL;
BLECharacteristic* pUartTxCharacteristic = NULL;
BLECharacteristic* pUartRxCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;
bool fNotify=1;
volatile bool fRx=0;
// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// FTMS (Fitness Machine Service)
static BLEUUID FTMS_SERVICE_UUID("1826");  // Standard 16-bit UUID for FTMS
static BLEUUID FTMS_TREADMILL_CHAR_UUID("2ACD");  // Treadmill Data Characteristic (notify)

// Nordic UART Service (NUS)
static BLEUUID UART_SERVICE_UUID("49535343-FE7D-4AE5-8FA9-9FAFD205E455");
static BLEUUID UART_RX_CHAR_UUID("49535343-8841-43F4-A8D4-ECBE34729BB3");
static BLEUUID UART_TX_CHAR_UUID("49535343-1E4D-4BD9-BA61-23C647249616");


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      fNotify=1;
      clear_cmd_resp();
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {

      deviceConnected = false;
    }
};
typedef struct {
  const char *cmd;
  const char *resp;  
  const char *cmd_info;
  int send_once;          // tristate, 0 = send, 1=send_once, -1=nosendmore....
} Cmd_Resp;

#if 0 
void convert_WorkoutData(const uint8_t *data, struct MessageWorkoutData *wd)
{
  // data[]==0x06
 	wd->Minute = data[0];
	wd->Second = data[1];
	wd->Distance = (data[2] << 8) | data[3];
	wd->Calories = (data[4] << 8) | data[5];
	wd->HeartRate = data[6];
	wd->Speed = data[7];
	wd->Incline = data[8];
	wd->HRType = data[9];
	wd->IntervalTime = data[10];
	wd->RecoveryTime = data[11];
	wd->ProgramRow = data[12];
	wd->ProgramColumn = data[13];
}

//        0  1  2    4    6  7  8   9   10  11  12  13
//        MM SS DIST CAL  HR SP INC HRT INT REC ROW COL
//5b0f 06 09 3b 0000 0000 00 05 00  00  00  00  00  01  5d

// Test 20250609 with these values:
//
//5b0f 06 09 3b 1111 0123 40 05 02  00  00  00  00  01  5d  // 
#endif

Cmd_Resp cmd_resp[] = {
  { "5b01f05d",           "5b08f0930000dc0a0f125d", "x", 0 },
  { "5b060700230037aa5d",           "5b0400074f4b5d", "x", 0 },
  { "5b030810015d",           "5b0400084f4b5d", "x", 0 },
  { "5b0504000000005d",           "5b0400044f4b5d", "x", 0 },
/*
  { "5b01f05d",           "5b08f0930000dc0a0f125d", "Get Device Info", 0 },
  { "5b060700230155235d", "5b0400074f4b5d",      "User profile", 1 },
  { "5b060700230155235d", "5b0203015d",      "User profile", 0 },

  //{ "5b060700230037aa5d",   "5b0400074f4b5d",    "0x07 user profile", },
  { "5b060700230037aa5d",   "5b0203015d",        "0x07 user profile", 0 },

  { "5b0203015d", "5B0203015D",      "0x03 User profile", 0 },
  { "5b030810015d",       "5b0203015d",         "eff!", 0 },
  { "5b0504000000005d",   "5b0203015d",        "0x04 experiemntal", 0 },
  { "5b0400024f4b5d",   "5B0203015D",        "0x02 workoutmode??", 0 },
  { "5b0400064f4b5d",   "5b0f06093b1211012340050200000000015d",        "0x06 get workout data", 0 },
  { "5b02220a5d",   "5b0203015d",    "0x22 set workout mode", 0 },
  { "5b0202025d",   "5b0203015d",  "set workout_manual", 0 },
  { "5b0400224f4b5d",   "5b0203015d",   "Unverified  responses", 0 },*/
  { NULL,NULL,NULL, },
};


//uint8_t notifydata[] = { 0xbc, 0x05, 0xc8, 0x00, 0xc8, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x85, 0x02 };
void clear_cmd_resp(void)
{
    for ( int cnt = 0; NULL != cmd_resp[cnt].resp  ; cnt++ ) {
      if ( -1 == cmd_resp[cnt].send_once )
        cmd_resp[cnt].send_once=1;
    }
}

void uartTxSendStr( const char *str)
{
  uint8_t resp[128];
  Serial.printf("TX: %s\n",str);
  hexStr2bytes(str,  resp, strlen(str));
  pUartTxCharacteristic->setValue(resp,strlen(str)/2); // HArdcoded assumption!
  pUartTxCharacteristic->notify();        

}
	
void parse_cmd_send_resp(uint8_t *buf, uint16_t len)
{
  bool found=0;
  char str[128];

  bytes2hexStr(buf, len, str);
  // Test strange behaviour of F85
  if (0x00 == strcmp("5b01f05d", str) ) {
    fNotify = false;
  }
  for ( int cnt = 0; cmd_resp[cnt].cmd != NULL; cnt++) {
    if ( 0x00 == strcmp(cmd_resp[cnt].cmd, str) && (cmd_resp[cnt].send_once >=0) ) {
      if ( 1 == cmd_resp[cnt].send_once ) {
        Serial.printf("!!!!");
        cmd_resp[cnt].send_once = -1;
      }
      found=1;
      uartTxSendStr(cmd_resp[cnt].resp);
      break;
    }
  }
  if ( !found) {
      Serial.printf("!");
        uartTxSendStr(str);
  }
}

//uint8_t tx1[] = { 0x5B, 0x08, 0xF0, 0x92, 0x00, 0x01, 0x78, 0x05, 0x0F, 0x12, 0x5D};
uint8_t recv_buf[128];
uint16_t recv_len;
uint8_t txValue = 0;
class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String rxValue = pCharacteristic->getValue();


    if (rxValue.length() > 0) {
      Serial.print("RX: ");
      fRx=0;
      recv_len = rxValue.length();
      for (int i = 0; i < recv_len; i++) {
        Serial.printf("%02x",rxValue[i]);
        recv_buf[i]=rxValue[i];
      }
      Serial.println();
      fRx=1;
    }
  }
};


void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.printf("Starting\n");
  // Create the BLE Device
  BLEDevice::init("SOLE");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());


  // Create the BLE FMTS Service
  BLEService *pFmtsService = pServer->createService(FTMS_SERVICE_UUID);


  // Create a BLE Characteristic
  pFmtsCharacteristic = pFmtsService->createCharacteristic(
                      FTMS_TREADMILL_CHAR_UUID,
                      BLECharacteristic::PROPERTY_READ    |
                      //|
                      //BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY
                      //BLECharacteristic::PROPERTY_INDICATE
                    );

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pFmtsCharacteristic->addDescriptor(new BLE2902());

/**********************/
/**********************/

  // Create the BLE UART Service
  BLEService *pUartService = pServer->createService(UART_SERVICE_UUID);

  // Create a BLE Characteristic
  pUartTxCharacteristic = pUartService->createCharacteristic(
                      UART_TX_CHAR_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      //BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY
                      //BLECharacteristic::PROPERTY_INDICATE
                    );
  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pUartTxCharacteristic->addDescriptor(new BLE2902());

  // Create a BLE Characteristic
  pUartRxCharacteristic = pUartService->createCharacteristic(
                      UART_RX_CHAR_UUID,
                      //BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE
                      //BLECharacteristic::PROPERTY_NOTIFY
                      //BLECharacteristic::PROPERTY_INDICATE
                    );
  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pUartRxCharacteristic->addDescriptor(new BLE2902());
  
  pUartRxCharacteristic->setCallbacks(new MyCallbacks());


  // Start the service(s)
  pFmtsService->start();
  pUartService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(FTMS_SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
}

uint8_t notifydata1[] = { 0xbc, 0x05, 0xc8, 0x00, 0xc8, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x85, 0x02 };
uint8_t notifydata[] = { 0xbc, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
void loop() {
    // notify changed value
    if (deviceConnected) {
      if ( fRx ) {
        fRx=0;
        parse_cmd_send_resp(recv_buf,  recv_len);
      }
      if ( fNotify && (value > 700) ) {

        notifydata[sizeof(notifydata)-2] = value;
        Serial.printf("\nnotify!!");
        for ( int cnt=0; cnt < sizeof(notifydata); cnt++) {
          Serial.printf("%02x", notifydata[cnt]);
        }
        Serial.printf("\n");
        pFmtsCharacteristic->setValue((uint8_t*)&notifydata, sizeof(notifydata));
        pFmtsCharacteristic->notify();
        value = 0;
      }
      value++;
      delay(3); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
    }
    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
}