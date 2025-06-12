#ifndef CMDS_H_
#define CMDS_H_

// This does not represent the internal notifiation structure!
typedef struct {
  // F85 notifications
  int time_i;   // Seconds
  float distance_f; // 
  float speed_f; //
  int incline_i; // 
  int calories_i; 
  int heartRate_i;
} FTMS;

void _recv_ftms_notification(uint8_t *data, uint16_t len);
int ftms_notification_get(FTMS *pFtms );
int ftms_hasNotify(void );
void _recv_cmd_notification( uint8_t *buf, uint16_t len);
int cmds_hasRx(void);
int cmds_get_response( uint8_t *buf, uint16_t max_len, uint16_t *len);
void send_command(uint8_t *cmd_buf, uint16_t len);
void send_command_hex(const char *cmd_string);
char *assemble_send_command(uint8_t cmd, uint8_t *data, int datalen);
int wait_response(uint8_t *buf, uint16_t *len, uint16_t timeout);
int wait_response_hex(char *resp_str, uint16_t timeout );
void convert_WorkoutData(const uint8_t *data, struct MessageWorkoutData *wd);
void print_workoutData(struct MessageWorkoutData *wd);
void convert_MessageDeviceInfo(const uint8_t *data, struct  MessageDeviceInfo *di);
void print_MessageDeviceInfo( struct  MessageDeviceInfo *di);
void print_MessageHeartRateType( struct MessageHeartRateType *hr);
void print_MessageUserProfile( uint8_t *data );
int analyze_resp(uint8_t *cmd_buf, int len, int *cmd, int *ack_type);


void analyze_print_resp(uint8_t *cmd_buf, int len);
void test_analyze(void);
void print_ErrorCode(uint8_t *buf);
void cmds_showInfo(uint8_t cmd, uint8_t *cmd_buf, uint16_t len);

void cmds_printBuf(uint8_t *buf, uint16_t len);
void cmds_init(void);


#define DeviceModel(x) x
#define UnitsType(x) x
#define Distance(x) ((float)x/10.0)
#define Speed(x) ((float)x/10.0)




#if 0
#define Speed byte
#define Weight uint16_t
#define Height byte
#define Program uint16
#define  DeviceModel byte
#define UnitsType byte
//#define WorkoutMode byte
#define SexType byte
#define MessageType byte
#define CommandType byte

#endif
enum {
  SexTypeMale = 0x01,
	SexTypeFemale   = 0x02,
};

enum {
	CommandTypeStart       = 0x01,
	CommandTypeLevelUp     = 0x02,
	CommandTypeLevelDown   = 0x03,
  CommandTypeInclineUp   = 0x04,
  CommandTypeInclineDown = 0x05,
	CommandTypeStop        = 0x06,
};


enum {
	ProgramManual    = 0x1001, //0x10 0x01
	ProgramHill      = 0x2002, //0x20 0x02
	ProgramFatBurn   = 0x2003, //0x20 0x03
	ProgramCardio    = 0x2004, //0x20 0x04
	ProgramStrength  = 0x2005, //0x20 0x05
	ProgramInterval  = 0x2006, //0x20 0x06
	ProgramHR1       = 0x3009, //0x30 0x09
	ProgramHR2       = 0x300a, //0x30 0x0a
	ProgramUser1     = 0x3007, //0x30 0x07
	ProgramUser2     = 0x3008, //0x30 0x08
	ProgramFusion    = 0x600c, //0x60 0x0c
};

enum {
	WorkoutModeIdle     = 0x01,
	WorkoutModeStart    = 0x02,
	WorkoutModeRunning  = 0x04,
	WorkoutModePause    = 0x06,
	WorkoutModeDone     = 0x07,
};
//| Name               | Code   | Request                                  | Response                 | ACK Type     | Direction         |
//|--------------------|--------|------------------------------------------|--------------------------|--------------|-------------------|
#define Acknowledge    0x00 //` | N/A                                      | `5b04 00 094f4b5d`         | None         | Both              |
#define SetWorkoutMode 0x02 //` | `5B02 02 025D`                           | `5B02 02 025D`             | Echo         | Host -> Treadmill |
#define WorkoutMode    0x03 //` | `5B02 03 015D`                           | `5B02 03 015D`             | Echo         | Treadmill -> Host |
#define WorkoutTarget  0x04 //` | `5B05 04 0A0000005D`                     | `5b04 0004 4f4b5d`         | ACK          | Host -> Treadmill |
#define WorkoutData    0x06 //` | `5b0f 06 093b0000000000050000000000015d` | `5b04 0006 4f4b5d`         | ACK          | Treadmill -> Host |
#define UserProfile    0x07 //` | `5B06 07 0123009B435D`                   | `5b04 0007 4f4b5d`         | ACK          | Host -> Treadmill |
#define ProgramType    0x08 //` | `5b03 08 10015d`                         | `5b04 0008 4f4b5d`         | ACK          | Host -> Treadmill |
#define HeartRateType  0x09 //` | `5b03 09 01005d`                         | `5b04 0009 4f4b5d`         | ACK          | Treadmill -> Host |
#define ErrorCode      0x10 //` | `5b02 10 005d`                           | `5b04 0010 4f4b5d`         | ACK          | Treadmill -> Host |
#define EndWorkout     0x32 //` | `5b0a 32 0013000000000800005d`           | `5b04 0032 4f4b5d`         | ACK          | Treadmill -> Host |
#define GetDeviceInfo  0xF0 //` | `5B01 F0 5D`                             | `5B08 F0 92000178050F125D` | Echo (Kinda) | Host -> Treadmill |
#define CommandType    0xF1 //` | `5B02 F1 xx 5D`                          | MessagTypweCommand         | ACK?         | Host -> Treadmill |
#define UNKNOWN        0xFFFFF

typedef enum {
	MessageTypeACK             = 0x00,
	MessageTypeSetWorkoutMode  = 0x02,
	MessageTypeWorkoutMode     = 0x03,
	MessageTypeWorkoutTarget   = 0x04,
	MessageTypeWorkoutData     = 0x06,
	MessageTypeUserProfile     = 0x07,
	MessageTypeProgram         = 0x08,
	MessageTypeHeartRateType   = 0x09,
	MessageTypeErrorCode       = 0x10,
	MessageTypeSpeed           = 0x11,
	MessageTypeIncline         = 0x12,
	MessageTypeLevel           = 0x13,
	MessageTypeRPM             = 0x14,
	MessageTypeHeartRate       = 0x15,
	MessageTypeTargetHeartRate = 0x20,
	MessageTypeMaxSpeed        = 0x21,
	MessageTypeMaxIncline      = 0x22,
	MessageTypeMaxLevel        = 0x23,
	MessageTypeUserIncline     = 0x25,
	MessageTypeUserLevel       = 0x27,
	MessageTypeEndWorkout      = 0x32,
	MessageTypeProgramGraphics = 0x40,
	MessageTypeDeviceInfo      = 0xf0,
	MessageTypeCommand         = 0xf1,
	MessageTypeUnknown         = 0xff,
} CMD_RESP_MSGS ;

enum ACK_Types {
  None=0,
  Echo = 1,
  Echo_kinda=2,
  Ack = 3,
};
struct MessageDeviceInfo {
	uint8_t Model;
	uint8_t Version;
	uint8_t Units;
	uint8_t MaxSpeed;
	uint8_t MinSpeed;
	uint8_t InclineMax;
	uint8_t UserSegment;
};

	
struct MessageWorkoutData {
  uint8_t Minute; //
  uint8_t	Second;
  uint16_t	Distance;
  uint16_t	Calories;
  uint8_t	HeartRate;
  uint8_t	Speed;
  uint8_t	Incline;
  uint8_t	HRType;
  uint8_t	IntervalTime;
  uint8_t	RecoveryTime;
  uint8_t	ProgramRow;
  uint8_t	ProgramColumn;
};

struct MessageEndWorkout {
  uint16_t	Seconds;
  uint16_t	Distance;
  uint16_t	Calories;
  uint8_t Speed;
  uint8_t	HeartRate;
  uint8_t	Incline;
};

struct MessageProgramGraphics {
	uint8_t Graph [18];
};

struct MessageHeartRateType {
	uint8_t Type1; 
	uint8_t Type2;
};

#if 0
type MessageCommand struct {
	Command CommandType;
}

type MessageProgram struct {
	Program Program;
}
struct MessageWorkoutMode  {
	Mode WorkoutMode;
}
type MessageSetWorkoutMode struct {
	Mode WorkoutMode
}

struct MessageUserProfile {
SexType	Sex;
byte	Age;
Weight	Weight;
Height	Height;
}
struct MessageACK  {
	Acknowledged MessageType
};

struct MessageWorkoutTarget  {
	Time     byte
	Calories uint16
};
struct MessageMaxIncline struct {
	MaxIncline byte
};
struct MessageErrorCode  {
	Code byte
};
struct MessageSpeed  {
	Speed Speed
};
struct MessageIncline struct {
	Incline byte
};
struct MessageLevel struct {
	Level byte
};
struct MessageRPM struct {
	RPM byte
};
struct MessageHeartRate  {
	HeartRate byte
};
struct MessageTargetHeartRate  {
	HeartRate byte
};
struct MessageMaxSpeed  {
	Speed Speed
};
struct MessageMaxLevel  {
	Level byte
};
struct MessageUserIncline  {
	Incline byte
};
struct MessageUserLevel  {
	Level byte
};
#endif // 
#endif // CMDS_H_