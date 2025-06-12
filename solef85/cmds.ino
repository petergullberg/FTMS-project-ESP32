#include <stdint.h>
#include <string.h>
#include "ble.h"
#include "cmds.h"
#include "util.h"
//util.ino

static volatile int fRx;
static uint8_t recv_buf[128];
static uint16_t recv_len;
static uint32_t ftms_time; // Last time we saw a ftms
static volatile bool ftms_notificationflag; // flat tnat 
static FTMS ftms;

// Ble stack calls this
// We just assume that we receive 24bytes of data.
void _recv_ftms_notification(uint8_t *data, uint16_t len)
{
  // F85 notifications
  if ( len < 24 ) {
    Serial.printf("oopos\n");
  }
  ftms.time_i = (data[23] << 8) | data[22];
  ftms.distance_f = ((data[5] << 8) | data[4] ) / 1000.0;
  ftms.speed_f = ((data[3] << 8) | data[2] ) / 10.0;
  ftms.incline_i =  data[7] / 10;
  ftms.calories_i = (data[17] << 8) | data[16];
  ftms.heartRate_i = data[10]; //???
  if ( fShowNotification ) {
    Serial.printf("Time: %d sec, Distance: %.1f km, Speed: %.1f km/h, Incline: %d%%, Calories: %d, HR: %d bpm\n",
                  ftms.time_i, ftms.distance_f, ftms.speed_f, ftms.incline_i, ftms.calories_i, ftms.heartRate_i);
  }
  ftms_time=millis();
  ftms_notificationflag=1;
}
int ftms_notification_get(FTMS *pFtms ) {
  if ( nullptr != pFtms ) {
    memcpy( pFtms, &ftms, sizeof(ftms) );
    return 1;
  }
  else {
    return 0;
  }
}

int ftms_hasNotify(void )
{
  return ftms_notificationflag;
}

void _recv_cmd_notification( uint8_t *buf, uint16_t len)
{
  memcpy(recv_buf, buf, len);
  recv_len =len;
#if 0
  Serial.printf("RX: ");
  for (int cnt = 0; cnt < len; cnt++) {
    Serial.printf("%02X ", recv_buf[cnt]);           // print raw data to uart
  }
  Serial.printf("\n");
#endif  

  fRx = 1;
}
int cmds_hasRx(void)
{
  return fRx;
}
// Returns 

int cmds_get_response( uint8_t *buf, uint16_t max_len, uint16_t *len)
{

  *len = 0;

  if ( fRx && (recv_len < max_len)  ) {
    memcpy((void *)buf, (const void *)recv_buf, (size_t) recv_len);
    *len = recv_len;
  }

  fRx = 0; // for all reasons!
  return *len;
}


void send_command(uint8_t *cmd_buf, uint16_t len)
{
  printf("TX: ");
  for ( int cnt = 0; cnt < len; cnt ++ ) {
    printf("%02x", cmd_buf[cnt]);
  }
  printf("\n");
  fRx=0; // Clear out response (risk that we get periodic failures anyway!)
   _ble_send_command(cmd_buf, len);
}

void send_command_hex(const char *cmd_string)
{
  int len;
  uint8_t  cmd_buf[128];

  len = strlen(cmd_string);
  hexStr2bytes(cmd_string, cmd_buf, len);
  send_command(cmd_buf, len/2);
}

char *assemble_send_command(uint8_t cmd, uint8_t *data, int datalen)
{
  static uint8_t cmd_buf[128];
  
  cmd_buf[0] = 0x5b;
  cmd_buf[1] = cmd;
  memcpy(&cmd_buf[2], data, datalen);
  cmd_buf[2+datalen] = 0x5d;
  send_command( cmd_buf, datalen+3);
  return nullptr;
}

// timeout in millseconds
int wait_response(uint8_t *buf, uint16_t *len, uint16_t timeout)
{
  uint16_t _timeout_cnt = timeout;

  *len=0;

  while ( _timeout_cnt ) {
    if ( fRx ) {
      memcpy(buf,recv_buf, recv_len);
      *len=recv_len;
      fRx=0;
      return 1;
    }
    delay(1);
    _timeout_cnt--;
  }
  return 0;
}

int wait_response_hex(char *resp_str, uint16_t timeout )
{
  uint16_t len;
  uint8_t  cmd_buf[128];
  
  resp_str[0]=0x00;
  
  if ( wait_response(cmd_buf, &len, timeout) ) {
    bytes2hexStr(cmd_buf, len, resp_str );
    return 1;
  }
  else {
    return 0;
  }
}

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

void print_workoutData(struct MessageWorkoutData *wd)
{
 	printf("MM:SS:    %02d:%02d \n", wd->Minute, wd->Second);
  printf("Distance: %d\n", wd->Distance );
	printf("Calories: %d\n", wd->Calories );
	printf("HR:       %d\n", wd->HeartRate );
	printf("Speed:    %2.1f\n", Speed( wd->Speed ) );
	printf("Incline   %d\n", wd->Incline );
	//printf("wd->HRType );
	printf("Interval: %d\n", wd->IntervalTime );
	//printf("wd->RecoveryTime );
	//printf("wd->ProgramRow  );
	//printf("wd->ProgramColumn );

}

void convert_MessageDeviceInfo(const uint8_t *data, struct  MessageDeviceInfo *di)
{
  // data[0] = 07
	di->Model = data[1];
	di->Version = data[2];
	di->Units = data[3];
	di->MaxSpeed = data[4];
	di->MinSpeed = data[5];
	di->InclineMax = data[6];
	di->UserSegment = data[7];
}

void print_MessageDeviceInfo( struct  MessageDeviceInfo *di)
{
  Serial.printf("MessageDeviceInfo\n");
	Serial.printf("DeviceModel %02x\n", DeviceModel(di->Model));
	Serial.printf("Version %d\n",di->Version);
	Serial.printf("DeviceModel %d\n",UnitsType(di->Units) );
	Serial.printf("Maxspeed %2.1f\n", Speed(di->MaxSpeed ) );
	Serial.printf("Min speed  %2.1f\n", Speed(di->MinSpeed) );
	Serial.printf("InclineMax  %d\n",di->InclineMax   );
	Serial.printf("Usersegment %d\n",di->UserSegment );
}

void print_MessageHeartRateType( struct MessageHeartRateType *hr)
{
  //Serial.printf("Heart rate: \n");
	Serial.printf("HR-type1 %d, ", DeviceModel(hr->Type1));
	Serial.printf("HR-type2 %d\n",hr->Type2);
}

void print_MessageUserProfile( uint8_t *data )
{
  Serial.printf("Sex:%d\n", data[1]);
	Serial.printf("p.Age = %d\n", data[2]);
	Serial.printf("p.Weight = %d\n", ((data[3]) << 8) | data[4]);
	Serial.printf("p.Height = %d\n", data[5]);
}

int analyze_resp(uint8_t *cmd_buf, int len, int *cmd, int *ack_type)
{
  *ack_type=None;
  *cmd=UNKNOWN;
  if ( 0 == len ) {
    *ack_type = None;
    *cmd = UNKNOWN;
    return 0;
  }
  
  switch ( cmd_buf[0] ) { 
    case 0x00: {
      *cmd=Acknowledge;
      *ack_type=None;
      return 1;
    }
    case 0x5b:    // follow protocol
      break;
    default:
      *ack_type = None;
      *cmd = UNKNOWN;
      return 0;
  }
  if ( len > 3 ) {
    if ( 0x00 == cmd_buf[2] ) {   // RESP...
      switch ( cmd_buf[3] ) {
        case 0x04:
          *ack_type = Ack;
          *cmd = WorkoutTarget;
          return 1;
        case 0x06:
          *ack_type = Ack;
          *cmd = WorkoutData;   // Seems wrong in the spec
          return 1;
        case 0x07:
          *ack_type = Ack;
          *cmd = UserProfile;
          return 1;
        case 0x08:
          *ack_type = Ack;
          *cmd = ProgramType;
          return 1;
        case 0x09:  // Ambigious
          *ack_type = Ack;
          *cmd = HeartRateType;
          return 1;
        case 0x10:
          *ack_type = Ack;
          *cmd = ErrorCode;
          return 1;
        case 0x32:
          *ack_type = Ack;
          *cmd = EndWorkout;
          return 1;
        default:
          *ack_type = None;
          *cmd = UNKNOWN;
          return 0;
      }        
    }
    else {  // cmd > 0x00
      switch ( cmd_buf[2] ) {
        case 0x02:
          *ack_type = Echo;
          *cmd = SetWorkoutMode;
          return 1;
        case 0x03:
          *ack_type = Echo;
          *cmd = WorkoutMode;
          return 1;
        case 0x04:
          *ack_type = Ack;
          *cmd = WorkoutTarget;
          return 1;
        case 0x06:
          *ack_type = Ack;
          *cmd = WorkoutData;   // Seems wrong in the spec
          return 1;
        case 0x07:
          *ack_type = Ack;
          *cmd = UserProfile;
          return 1;
        case 0x08:
          *ack_type = Ack;
          *cmd = ProgramType;
          return 1;
        case 0x09:  // Ambigious
          *ack_type = Ack;
          *cmd = HeartRateType;
          return 1;
        case 0x10:
          *ack_type = Ack;
          *cmd = ErrorCode;
          return 1;
        case 0x32:
          *ack_type = Ack;
          *cmd = EndWorkout;
          return 1;
        case 0xF0:
          *ack_type = Echo_kinda;
          *cmd = GetDeviceInfo;
          return 1;
        default:
          *ack_type = None;
          *cmd = UNKNOWN;
          return 0;
      }
    }
  }
  *ack_type = None;
  *cmd = UNKNOWN;
  return 0;
}

void analyze_print_resp(uint8_t *cmd_buf, int len)
{
  int cmd,ack_type;
  Serial.printf("RX: ");
  for (  int cnt=0;cnt<len;cnt++) {
    Serial.printf("%02x",cmd_buf[cnt]);

  }
  Serial.printf("\n");
  analyze_resp(cmd_buf, len, &cmd, &ack_type);
  cmds_showInfo(cmd, cmd_buf, len);
}

typedef struct {
  int value;
  const char *ptr;
} Test_Values;
  
Test_Values test_values[]= {
{  Acknowledge, "00" },
{  SetWorkoutMode, "5B0202025D"},
{  WorkoutMode, "5B0203015D"},
{  WorkoutTarget, "5B05040A0000005D"},
{  WorkoutData, "5b0f06093b0000000000050000000000015d"},
{  UserProfile, "5B06070123009B435D"},
{  ProgramType, "5b030810015d"},
{  HeartRateType, "5b030901005d"},
{  ErrorCode, "5b0210005d"},
{  EndWorkout, "5b0a320013000000000800005d"},
{  GetDeviceInfo, "5B01F05D"},

{  Acknowledge, "5b0400094f4b5d"},
{  SetWorkoutMode, "5B0202025D"},
{  WorkoutMode, "5B0203015D"},
{  WorkoutTarget, "5b0400044f4b5d"},
{  WorkoutData, "5b0400064f4b5d"},
{  UserProfile, "5b0400074f4b5d"},
{  ProgramType, "5b0400084f4b5d"},
{  HeartRateType, "5b0400094f4b5d"},
{  ErrorCode, "5b0400104f4b5d"},
{  EndWorkout, "5b0400324f4b5d"},
{  GetDeviceInfo, "5B08F092000178050F125D"},
{  0x00, nullptr },
};

void test_analyze(void)
{
  uint8_t dest_buf[128], len;
  int ack_type, cmd;
  for ( int cnt = 0;  nullptr != test_values[cnt].ptr ; cnt++ ) {
    len = strlen(test_values[cnt].ptr);
    hexStr2bytes(test_values[cnt].ptr, dest_buf, len);
    analyze_resp(dest_buf, len/2, &cmd, &ack_type);
    printf("[%d]cmd=%02x, (len=%d) analyzed_cmd=%02x, acktype=%d, cmd: \"%s\"\n", 1+cnt, test_values[cnt].value, len, cmd, ack_type, test_values[cnt].ptr); 
  }

}
void print_ErrorCode(uint8_t *buf)
{
    Serial.printf("ErrorCode:%02x",buf[0]);

}

void cmds_showInfo(uint8_t cmd, uint8_t *cmd_buf, uint16_t len)
{
  int offset=2;

  if ((len>=3) && (0x00 == cmd_buf[2]) ) {
    offset = 3;
  }
  else { 
    offset = 2;
  }
  switch(cmd) {
		case Acknowledge:
			break;
		case SetWorkoutMode:
      Serial.printf("Set Workoutmode");
			break;
		case WorkoutMode:
      Serial.printf("Workoutmode: %d\n", cmd_buf[offset+1]);
			break;
		case WorkoutTarget:
			break;
		case WorkoutData:
      Serial.printf("WorkoutData");
      print_workoutData((struct MessageWorkoutData *) &cmd_buf[offset+1]);
			break;
		case UserProfile:
      Serial.printf("Userprofile");
			break;
		case ProgramType:
			break;
		case HeartRateType:  // 5b03 09 01005d`
      print_MessageHeartRateType( (struct MessageHeartRateType *) &cmd_buf[offset+1]);

			break;
		case ErrorCode:
      Serial.printf("ErrorCode\n");
      print_ErrorCode( &cmd_buf[offset+1]);
			break;
		case EndWorkout:
      Serial.printf("endworkout\n");
			break;
		case GetDeviceInfo:

      print_MessageDeviceInfo( (struct  MessageDeviceInfo *)& cmd_buf[offset+1]);
			break;
    default:
        break;
  }
}

void cmds_printBuf(uint8_t *buf, uint16_t len)
{
  char str[128];
  bytes2hexStr(buf, len, str );
  Serial.printf("%s\n",str);
}


void cmds_init(void)
{
  fRx=0;
  ftms_time = 0;
  ftms_notificationflag = false;
  
}
