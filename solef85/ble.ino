#if 1
/**
 * A BLE client example that is rich in capabilities.
 * There is a lot new capabilities implemented.
 * author unknown
 * updated by chegewara
 */

#include <stdio.h>
#include <stdint.h>
#include <BLEDevice.h>
#include "ble.h"
#include "cmds.h"

/*
https://github.com/cagnulein/qdomyos-zwift/blob/0f7b240f4a396cb588e6540f12c8f6645084afc6/src/devices/soleelliptical/soleelliptical.cpp#L71

*/
//#include "BLEScan.h"

// The characteristic of the remote service for UART
static BLEUUID serviceUUID("49535343-FE7D-4AE5-8FA9-9FAFD205E455");

// The characteristics of the above service we are interested in.
// The client can send data to the server by writing to this characteristic.
static BLEUUID charUUID_RX("49535343-8841-43F4-A8D4-ECBE34729BB3");

// If the client has enabled notifications for this characteristic,
// the server can send data to the client as notifications.
static BLEUUID charUUID_TX("49535343-1E4D-4BD9-BA61-23C647249616");

/* To get notifications on workout data*/

#define FMS_SERVICE        "1826"
#define TREADMILL_DATA     "2ACD" 

static BLERemoteCharacteristic* pTXCharacteristic;
static BLERemoteCharacteristic* pRXCharacteristic;
static BLERemoteCharacteristic* pFtmsCharacteristic;

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLEAdvertisedDevice *myDevice;
static char MAC_ADDRESS[20] = "";
#define MAC_DEFAULT "f5:c0:00:0b:c6:eb"

// Interface code
void _ble_send_command(uint8_t *cmd_buf, uint16_t len)
{
  pRXCharacteristic->writeValue(cmd_buf,(size_t) len);
}

int ble_is_connected(void)
{
    return connected;
}


bool fShowNotification=1;
void notifyCallback_FTMS(BLERemoteCharacteristic* pBLERemoteCharacteristic,
                    uint8_t* pData, size_t length, bool isNotify) {
  if (length < 11) {
    Serial.printf("!");
    return;
  } 

  _recv_ftms_notification(pData, length);
 
#if 0  // F80 remnants?
  int time = pData[0] | (pData[1] << 8);
  float distance = (pData[2] | (pData[3] << 8)) / 10.0;
  float speed = (pData[4] | (pData[5] << 8)) / 10.0;
  int incline = pData[6] | (pData[7] << 8);
  int calories = pData[8] | (pData[9] << 8);
  int heartRate = pData[10];

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
#endif

}





static void notifyCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify) {
#if 0
  Serial.print("Notify callback for characteristic ");
  Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
  Serial.print(" of data length ");
  Serial.println(length);
  Serial.print("data: ");
  Serial.write(pData, length);
  Serial.println();
#else
  _recv_cmd_notification(pData, length);
#endif
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient *pclient) {}

  void onDisconnect(BLEClient *pclient) {
// PEGE:
    pTXCharacteristic  = nullptr;
    pRXCharacteristic = nullptr;
    pFtmsCharacteristic = nullptr;

    connected = false;
    Serial.println("onDisconnect");
  }
};

bool connectToServer() {
  Serial.print("Forming a connection to ");
  Serial.println(myDevice->getAddress().toString().c_str());

  BLEClient *pClient = BLEDevice::createClient();
  Serial.println(" - Created client");

  pClient->setClientCallbacks(new MyClientCallback());

  // Connect to the remove BLE Server.
  pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
  Serial.println(" - Connected to server");
  pClient->setMTU(517);  //set client to request maximum MTU from server (default is 23 otherwise)


/**********************************************/
/**********************************************/
/**********************************************/
/**********************************************/

  Serial.printf("Connected to: %s RSSI: %d\n", pClient->getPeerAddress().toString().c_str(), pClient->getRssi());

  // Obtain a reference to the Nordic UART service on the remote BLE server.
  BLERemoteService* pRemoteService1 = pClient->getService(serviceUUID);
  Serial.printf("!");
  if (pRemoteService1 == nullptr) {
    Serial.print("Failed to find Nordic UART service UUID: ");
    Serial.println(serviceUUID.toString().c_str());
    return false;
  }
  Serial.println(" - Remote BLE service reference established");


  // Obtain a reference to the TX characteristic of the Nordic UART service on the remote BLE server.
  pTXCharacteristic = pRemoteService1->getCharacteristic(charUUID_TX);
  if (pTXCharacteristic == nullptr) {
    Serial.print("Failed to find TX characteristic UUID: ");
    Serial.println(charUUID_TX.toString().c_str());
    return false;
  }
  Serial.println(" - Remote BLE TX characteristic reference established");

  // Read the value of the TX characteristic.
  String value = pTXCharacteristic->readValue();
  //std::string value = pTXCharacteristic->readValue();
  Serial.print("The characteristic value is currently: ");
  Serial.println(value.c_str());


  pTXCharacteristic->registerForNotify(notifyCallback);
#if 0 // This is for NimBLE
  if(pTXCharacteristic && pTXCharacteristic->canNotify()) {
    Serial.printf("Can Notify\n");
    if(!pTXCharacteristic->subscribe(true, notifyCallback)) {
          Serial.printf("Failed to subscribe\n");

          /** Disconnect if subscribe failed */
          pClient->disconnect();
          return false;
      }      
      Serial.printf("Setting notification\n");
  }
#endif
  // Obtain a reference to the RX characteristic of the Nordic UART service on the remote BLE server.
  pRXCharacteristic = pRemoteService1->getCharacteristic(charUUID_RX);
  if (pRXCharacteristic == nullptr) {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(charUUID_RX.toString().c_str());
    return false;
  }
  Serial.println(" - Remote BLE RX characteristic reference established");

 
  BLERemoteService* pRemoteService2 = pClient->getService(FMS_SERVICE);
  if (pRemoteService2 == nullptr) {
    Serial.println("Could not find FNS service\n");
    return false;
  }

    pFtmsCharacteristic = pRemoteService2->getCharacteristic(TREADMILL_DATA);
  if (pFtmsCharacteristic && pFtmsCharacteristic->canNotify()) {
    pFtmsCharacteristic->registerForNotify(notifyCallback_FTMS);
    Serial.println("Subscribed to FMS WORKOUTDATA notifications");
  } else {
    Serial.println("WORKOUTDATA characteristic not found or can't notify");
  }
/*--------*/
  connected = true;
  return true;
}
/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    // We have found a device, let us now see if it contains the service we are looking for.
    //if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {
    BLEAddress thisAddress = advertisedDevice.getAddress();      
    if ( 0 == strcmp( thisAddress.toString().c_str() , "f5:c0:00:0b:c6:eb")  ) {
      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;

    }  // Found our server
  }  // onResult
};  // MyAdvertisedDeviceCallbacks

void test_analyze(void);





void ble_loop() {
  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are
  // connected we set the connected flag to be true.
  if (true == doConnect ) {

    if (connectToServer()) {
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println("We have failed to connect to the server; there is nothing more we will do.");
    }
    doConnect = false;
  }

  // If we are connected to a peer BLE Server, update the characteristic each time we are reached
  // with the current time since boot.
  if (connected) {
      // how often to notify...

  } else if (doScan) {
    BLEDevice::getScan()->start(0);  // this is just example to start scan after disconnect, most likely there is better way to do it in arduino
  }

  delay(5);
}  // End of loop

void ble_startPairing(void)
{
  //ble_pairing = true;
  //doClearCache = true;
  // PEGE Experimental to clear cache, when switching pariing, but only when scanning has completed.
}


void ble_setmac(const char *str)
{
  if ( strlen(str) < sizeof(MAC_ADDRESS) ) {
    strcpy(MAC_ADDRESS, str);
  }
  else {
     Serial.printf("ERRORsetting mac\n");
  }
}


void ble_setup() {

  if ( 0x00 == strlen(MAC_ADDRESS) ) {
      strcpy( MAC_ADDRESS, MAC_DEFAULT);
  }

  BLEDevice::init("SOLE F85 Client");

  doConnect = false;
  connected = false;
  doScan = false;
  //doClearCache = false;
  //fNotificationUpdate  = false;

  //Serial.printf("BLE power = %d\n", BLEDevice::getPower()); 
  BLEDevice::setPower(ESP_PWR_LVL_P9); /** +9db */
  
  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan *pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(30, false);
  
}  // End of setup.




#if 0
  if ( ble_fNotificationUpdate  ) {
      debug_I( TAG_BLE, "Temperature = %2.1f*C  Humidity %2.1f %RH\n", ble_temperature, ble_humidity );
      ble_fNotificationUpdate  = false;
    }

//
NimBLEScanResults scanResults = pScan->start(0); // <-- 0 means scan forever
Also change this line, because your beacons will be reported only one time:
//pScan->(new MyAdvertisedDeviceCallbacks(), true); // <-- true means you want duplicates, unless you dont

#endif

#endif