#include <stdint.h>
#include <BLEDevice.h>
#include "cmds.h"
#include "serial.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "solef80treadmill.h"
void sdcard_setup();
void sdcard_test(void);
/**/

// Function that creates a task.
static int taskCore = 1;
static uint8_t ucParameterToPass;
TaskHandle_t xHandle;
#define STACK_SIZE  20000
#define PRIORITY 0


// Task that serves ble and serial communication
void vTaskLoop( void * pvParameters )
{
  
  ble_setup();
  for (;;) {
      ble_loop();
      // Do other stuff
      serial_poll(200);
  }
   vTaskDelete(NULL);
  
}

void deleteTask(void)
{
  if( xHandle != NULL )  {
     vTaskDelete( xHandle );
  }
}


void task_init( void )
{

  //debug_I(TAG_BLE, "Create ble Task\n");
  xHandle = NULL;

  // Create the task, storing the handle.  Note that the passed parameter ucParameterToPass
  // must exist for the lifetime of the task, so in this case is declared static.  If it was just an
  // an automatic stack variable it might no longer exist, or at least have been corrupted, by the time
  // the new task attempts to access it.

  xTaskCreatePinnedToCore(
                    vTaskLoop,   /* Function to implement the task */
                    "BLE", /* Name of the task */
                    20000,      /* Stack size in words */
                    &ucParameterToPass,       /* Task input parameter */
                    PRIORITY, /* Priority of the task, higher the more important */
                    &xHandle,         /* Task handle. */
                    taskCore);    /* Core where the task should run */

  configASSERT( xHandle );

  //debug_I(TAG_BLE, "Task created\n");
  // Use the handle to delete the task.
}

static int _cnt = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Starting Arduino BLE Client application...");
  cmds_init();
  sdcard_setup();
  sdcard_test();
  task_init();    // Handle IO and BLE 

}  // End of setup.

static int do_cnt = 0;

// This handles commands/notifications from treadmill
void process_rcvd(void)
{
  uint8_t  buf[128];
  uint16_t len;
  // should actually be GET RESPONSE...
  if ( cmds_get_response(buf, sizeof(buf), &len) ) {
    int cmd, ack_type;
    if ( analyze_resp(buf, len, &cmd, &ack_type) ) {
      cmds_printBuf(buf,len);
      cmds_showInfo( cmd, buf, len);
      // unknown: 5b0400094f4b5d
      switch ( cmd ) {
        case WorkoutMode:  // from TM
          Serial.printf("WorkoutMode:");
          send_command_hex("5B0203015D");
          break;
        case HeartRateType: // from TM
          Serial.printf("Heartrate:");
          send_command_hex("5b0400094f4b5d");
          break;
        case WorkoutData:
          Serial.printf("workoutdata:");
          send_command_hex("5b0400064f4b5d");
          break;
        case ErrorCode:
          Serial.printf("error:");
          send_command_hex("5b0400104f4b5d");
          break;
        case EndWorkout:
          Serial.printf("end workout: ");
          send_command_hex("5b0400324f4b5d");
          break;
        default: // else
          Serial.printf("else:");
        
          send_command_hex("5b0400094f4b5d");
          break;
      }
    } else { 
      cmds_showInfo( cmd, buf, len);
      Serial.printf("Failed to decode response\n");
    }
  }
}

// Process serial command recieved from uar
void process_ser_cmd(void)
{
  uint8_t buf[128];
  uint16_t len;
  // We are supposed to have a line here...
  if ( serial_get_cmd(buf, sizeof(buf)-1, &len) ) {
    if ( ('P' == buf[0]) && ('G'==buf[1])) {
      switch (buf[2]) {
        case '0': break;
        case 'S': writeCharacteristic_hex( "5B01F05D", "serial_line", 0, false);
          break;
        case 'T': /*fShowNotification = ! fShowNotification;*/ break;
        case 'R': ESP.restart();
      }
      Serial.printf("Char set\n");
      //p2ACDCharacteristic->registerForNotify(notifyCallback_2ACD);
      //Serial.printf("%s\n", p2ACDCharacteristic->readValue().c_str());
    } else {
      writeCharacteristic_hex((const char *)buf, "serial_line", 0, true);
      if ( wait_response(buf, &len, 500) ) {
        int cmd, ack_type;

        if ( analyze_resp(buf, len, &cmd, &ack_type) ) {
          if ( WorkoutMode == cmd ) {
            //send_command(cmd_buf,len);
            Serial.printf(".");
          }
          else {
            cmds_printBuf(buf,len);
            cmds_showInfo( cmd, buf, len);
            Serial.printf("cmd:%02x ", cmd);
            switch ( ack_type ) {
              case None:
                Serial.printf(" ack_type:None\n");
                break;
              case Ack:
                Serial.printf(" ack_type:Ack\n");
                break;
              case Echo:
                Serial.printf(" ack_type:Echo\n");
                break;
              case Echo_kinda:
                Serial.printf(" ack_type:Echo_kinda\n");
                break;
            }
          }
        }
      }
    }
  }
}
// Elaborate structure to handle sequential proccess.. mainly for testing
void process_cnt( void )
{
  //delay(2000);
  uint8_t buf[128];
  char str[128];
  uint16_t len;
  switch (do_cnt) {
    case 0:
      break;
      // Test from go swedish
      writeCharacteristic_hex("5B01F05D", "get_device_info", 0, true);
      delay(2000);
      writeCharacteristic_hex("5b030810015d","MessageProgram 0x08 {ProgramManual, 0x1001}", 0, true);
      delay(2000);
      writeCharacteristic_hex("5B0202025D", "MessageSetWorkoutMode 0x02 {WorkoutModeStart 0x02}", 0, true);
      delay(2000);

      writeCharacteristic_hex("5b0400064f4b5d", "Starting exercise", 0, true);
      delay(500);

      //MessageProgram{ProgramManual}, MessageTypeACK); err != nil { 0x08
      //MessageWorkoutTarget{}, MessageTypeACK); err != nil {
      //MessageSetWorkoutMode{WorkoutModeStart}, MessageTypeSetWorkoutMode); err != nil {


      //btinit2();
      do_cnt=5;
      break;


      writeCharacteristic_hex("5B01F05D", "get_device_info", 0, true); // This allows us to communicate, but it stops notifications!
      writeCharacteristic_hex("5B0202045D","set workout", 0, true);  // returns RX: 5b 02 03 80 5d

      writeCharacteristic_hex("5b02f1045d", "up-incline", 0, true);  // 5b 04 00 f1 4f 4b 5d 
      writeCharacteristic_hex("5b02f1045d", "up-incline", 0, true);
      writeCharacteristic_hex("5b02f1045d", "up-incline", 0, true);
      writeCharacteristic_hex("5b02f1045d", "up-incline", 0, true);
      writeCharacteristic_hex("5b02f1045d", "up-incline", 0, true);
      writeCharacteristic_hex("5b02f1045d", "up-incline", 0, true);
      writeCharacteristic_hex("5b02f1045d", "up-incline", 0, true);

      writeCharacteristic_hex("5b02f1025d", "up-speed", 0, true);
      delay(1000);
      writeCharacteristic_hex("5b02f1025d", "up-speed", 0, true);
      delay(1000);
      writeCharacteristic_hex("5b02f1025d", "up-speed", 0, true);
      delay(1000);
      writeCharacteristic_hex("5b02f1025d", "up-speed", 0, true);
      delay(1000);
      writeCharacteristic_hex("5b02f1025d", "up-speed", 0, true);
      delay(1000);
      writeCharacteristic_hex("5b02f1025d", "up-speed", 0, true);
      delay(1000);
      writeCharacteristic_hex("5b02f1025d", "up-speed", 0, true);
      delay(1000);
      writeCharacteristic_hex("5b02f1025d", "up-speed", 0, true);
      delay(1000);
      writeCharacteristic_hex("5b02f1025d", "up-speed", 0, true);
      delay(1000);




      //uint8_t speed_up[] = {0x5b, 0x02, 0xf1, 0x02, 0x5d};
      //uint8_t speed_down[] = {0x5b, 0x02, 0xf1, 0x03, 0x5d};

      //uint8_t incline_up[] = {0x5b, 0x02, 0xf1, 0x04, 0x5d};
      //uint8_t incline_down[] = {0x5b, 0x02, 0xf1, 0x05, 0x5d};

      //writeCharacteristic_hex("5b02f10025d", "up-speed", 0, true);
      do_cnt=6;
      break;

      //solef80treadmill_btinit();
      // no increment
      //do_cnt=4;
      //break;

      printf("\n");

      // According to void solef80treadmill::btinit()
      //1 "5b01f05d"
      //2 "5b0203015d"
      //3 "5b0400094f4b5d"

      send_command_hex("5B01F05D");

      if ( wait_response(buf,&len,1000) ) {
        cmds_showInfo(GetDeviceInfo, buf, len);
        cmds_printBuf(buf,len);

        Serial.printf("Success\n");
        do_cnt++;
      }
      else {
        Serial.printf("fail");
        do_cnt=0;
      }
      break;
    case 1:
      send_command_hex("5b0203015d"); // set workout ode
      if ( wait_response_hex(str,1000) ) {
        Serial.printf("Success\n");
        do_cnt++;
      }
      else {
        Serial.printf("fail");
        do_cnt=0;
      }
      break;

    case 2:
    do_cnt++; break;

      send_command_hex("5b0400094f4b5d"); 
      if ( wait_response_hex(str,1000) ) {
        Serial.printf("Success\n");
        do_cnt++;
      }
      else {
        Serial.printf("fail");
        do_cnt=0;
      }
      break;

    case 3:
    do_cnt++; break;



      printf("Starting exercise\n"); // set workout ode
      send_command_hex("5b0400064f4b5d");
      if ( wait_response(buf,&len,1000) ) {
        Serial.printf("Success\n");
        do_cnt++;
      }
      else {
        Serial.printf("fail");
        do_cnt=0;
      }
      break;
    case 4:
    
      writeCharacteristic_hex("5B01F05D", "getdeviceinfo", 0, true);

      //writeCharacteristic_hex("5b0400064f4b5d", "workoutdata_resp!!?", 0, true);
      
      writeCharacteristic_hex("5b01065d", "command workout data", 0, true);
      delay(1000);
      //writeCharacteristic_hex("5b0400104f4b5d", "noop", 0, true);
      //delay(1000);
      //uint8_t noop[] = {0x5b, 0x04, 0x00, 0x10, 0x4f, 0x4b, 0x5d};
      //writeCharacteristic_hex("5b0400064f4b5d", "noop2", 0, true);
      //delay(1000);
      do_cnt++;
    
    // do nothing...
      break;

    case 5:
      _cnt++;
      if ( _cnt==1000) {
        //writeCharacteristic_hex("5b02f1065d", "MessageTypeCommand{CommandTypeStop}", 0, true);
        //writeCharacteristic_hex("5b0a320013000000000800005d", "end workout", 0, true);
      }

      //do_cnt=0;
      break;
    default:
      Serial.printf(".");
      break;
  }
}
// This is the Arduino main loop function.
void loop() {

  _cnt =0;
  while ( ble_is_connected() ) {
    // If we are connected to a peer BLE Server, update the characteristic each time we are reached
    // with the current time since boot.

    if ( serial_is_cmd() ) {
      process_ser_cmd();
    }

    if ( cmds_hasRx() ) {
      process_rcvd();
    }
    process_cnt();
  }


}  // End of loop
