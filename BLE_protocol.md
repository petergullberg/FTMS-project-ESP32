# SOLE F85 Bluetooth LE Technical Details - work in progress
- This original documentation came from swedishborgie
The treadmill appears to use a fairly common integrated BLE to UART module. 
The treadmill does not listen for connections while in low power mode, the display must be active in order to connect.

To perform serial communication you'll subscribe to notifications for the read characteristic to receive messages from
the treadmill, and you'll write messages to the write characteristic when you want to communicate.


# USB-Serial
- https://github.com/avinabmalla/ESP32_BleSerial/tree/master/src


It advertises the following:
- Service ID: 49535343-FE7D-4AE5-8FA9-9FAFD205E455
- The service exposes two different characteristics that are required to communicate:

- Read (Notify) - 49535343-1E4D-4BD9-BA61-23C647249616
- Write - 49535343-8841-43F4-A8D4-ECBE34729BB3

To perform serial communication you'll subscribe to notifications for the read characteristic to receive messages from the treadmill, and you'll write messages to the write characteristic when you want to communicate.

# Protocol
```
CMD:  5B | LEN_DATA | CMD | DATA | 5D
RESP: 5B | LEN_DATA | CMD | DATA | 5D
```
It appears that some responses requires certain CMD to be 2B, it's not clear why, the format is then "0009"

The messages sent back and forth between the treadmill use a fairly straight forward serial protocol:
 * All messages begin with `0x5b` followed immediately by a byte indicating the message length. All messages end with `0x5d`.
 * Most messages seem to require an acknowledgement and there are two types: 
   - an ACK message (`0x00`) or 
   - repeating the received command back to the treadmill.
   - [PG] or sennding a dedicated response for that group, or ack with "OK"
 * The first command you send should be `Get Device Info (0xf0)` which will result in the treadmill establishing
   communication.
 * Some messages are sent from the treadmill without any prompting from the host. These messages need to be acknowledged
   for communication to continue.
 * The treadmill cannot handle messages too quickly, even if messages have been promptly acknowledged. A short sleep (300-500ms)
   is usually sufficient between writes to let the treadmill catch up.
 * [PG] failing to properlyu acknowledge commands on F85, will stop workout characteristics.
 

## Messages
```
| Name              | URL   | Code   | Request                                | Response                 | ACK Type     | Direction         |
|-------------------------|--------|----------------------------------------|--------------------------|--------------|-------------------|
| [Acknowledge]     | [1]   | `0x00` | N/A                                    | `5b0400094f4b5d`         | None         | Both              |
| [Set Workout Mode]| [2]   | `0x02` | `5B0202025D`                           | `5B0202025D`             | Echo         | Host -> Treadmill |
| [Workout Mode]    | [3]   | `0x03` | `5B0203015D`                           | `5B0203015D`             | Echo         | Treadmill -> Host |
| [Workout Target]  | [4]   | `0x04` | `5B05040A0000005D`                     | `5b0400044f4b5d`         | ACK          | Host -> Treadmill |
| [Workout Data]    | [5]   | `0x06` | `5b0f06093b0000000000050000000000015d` | `5b0400064f4b5d`         | ACK          | Treadmill -> Host |
| [User Profile]    | [6]   | `0x07` | `5B06070123009B435D`                   | `5b0400074f4b5d`         | ACK          | Host -> Treadmill |
| [Program Type]    | [7]   | `0x08` | `5b030810015d`                         | `5b0400084f4b5d`         | ACK          | Host -> Treadmill |
| [Heart Rate Type] | [8]   | `0x09` | `5b030901005d`                         | `5b0400094f4b5d`         | ACK          | Treadmill -> Host |
| [Error Code]      | [9]   | `0x10` | `5b0210005d`                           | `5b0400104f4b5d`         | ACK          | Treadmill -> Host |
| [End Workout]     | [10]  | `0x32` | `5b0a320013000000000800005d`           | `5b0400324f4b5d`         | ACK          | Treadmill -> Host |
| [Get Device Info] | [11]  | `0xF0` | `5B01F05D`                             | `5B08F092000178050F125D` | Echo (Kinda) | Host -> Treadmill |

URL
[1] https://github.com/swedishborgie/treadonme/blob/04766d37a14b0fe02de2ea07d837dd8bc7e7d908/messages.go#L353-L392)     
[2] https://github.com/swedishborgie/treadonme/blob/04766d37a14b0fe02de2ea07d837dd8bc7e7d908/messages.go#L165-L193)
[3] https://github.com/swedishborgie/treadonme/blob/04766d37a14b0fe02de2ea07d837dd8bc7e7d908/messages.go#L135-L163)    
[4] https://github.com/swedishborgie/treadonme/blob/04766d37a14b0fe02de2ea07d837dd8bc7e7d908/messages.go#L394-L431)  
[5] https://github.com/swedishborgie/treadonme/blob/04766d37a14b0fe02de2ea07d837dd8bc7e7d908/messages.go#L226-L306)    
[6] https://github.com/swedishborgie/treadonme/blob/04766d37a14b0fe02de2ea07d837dd8bc7e7d908/messages.go#L308-L351)    
[7] https://github.com/swedishborgie/treadonme/blob/04766d37a14b0fe02de2ea07d837dd8bc7e7d908/messages.go#L918-L950)    
[8] https://github.com/swedishborgie/treadonme/blob/04766d37a14b0fe02de2ea07d837dd8bc7e7d908/messages.go#L195-L224) 
[9] https://github.com/swedishborgie/treadonme/blob/04766d37a14b0fe02de2ea07d837dd8bc7e7d908/messages.go#L463-L491)      
[10] https://github.com/swedishborgie/treadonme/blob/04766d37a14b0fe02de2ea07d837dd8bc7e7d908/messages.go#L793-L841)     
[11] https://github.com/swedishborgie/treadonme/blob/04766d37a14b0fe02de2ea07d837dd8bc7e7d908/messages.go#L83-L133)  
```


Notes:
* Speeds are specified in the units returned in the 
  [Get Device Info](https://github.com/swedishborgie/treadonme/blob/04766d37a14b0fe02de2ea07d837dd8bc7e7d908/messages.go#L83-L133)
  message and are specified in either kilometers per hour (for metric) or miles per hour (for imperial) and the value is
  multiplied by ten. So for instance if the units are imperial and the speed indicated is 62 the speed would be 6.2 miles per hour.
* Similarly, distances are specified in either kilometers or miles and are multiplied by one hundred. So if a distance of 102
  is returned and the units are imperial it would indicate a distance of 1.02 miles.

# MY ADDITIONAL NOTES:
* If I don't acknowledge the receied commands correctly in the uart characteristics, 
the workoutdata stops coming, and I'm not able 
* 4f4b "OK" seems to be a general "ack" / ok that is appended to responses from treadmill commands
* Apart from my nboties on F85, swedishborge has done a great job.

# References:
solef85

# SOLE F85 - 2019 WORKOUT_DATA
I haven't found a way to get workoutdata on the UART channel for F85, but I manage to get it via characteristics.

**FTMS GATT
The treadmill workout data on Sole F85 is recieved on a separate characteristics.
This this provide the relevant data during a workout
As noted above, the data is blocked by unacknowledge/wronglyacknowledged UART notifiations.

** Service and Characterstic
```
| ------------------- |------|------|
| Description         | INT  | TYPE |
| ------------------- |------| ---- |
| FMTS_SERVICE        | 1826 | S    |
| TREADMILL_DATA_CHAR | 2ACD | N    |
| ------------------- |------|----- |
```
To get the data, we need to setup notifications on the TREADMILL_DATA_CHAR.

The data we received looks something like this:
```
bc05c800c800002800000028000000030000000000008502 
```

I haven't fully decoded data, but here is the current bulk of it:
```
// F85 notifications
  int time = (pData[23] << 8) | pData[22];
  float distance = ((pData[5] << 8) | pData[4] ) / 1000.0;
  float speed = ((pData[3] << 8) | pData[2] ) / 10.0;
  int incline =  pData[7] / 10;
  int calories = (pData[17] << 8) | pData[16];
  int heartRate = pData[10]; //???
  
  Serial.printf("Time: %d sec, Distance: %.1f km, Speed: %.1f km/h, Incline: %d%%, Calories: %d, HR: %d bpm\n",
    time, distance, Speed(speed), incline, calories, heartRate);
```
My machine is apparently setup for km/h and km, haven't tried anything else.


# Other references:
This repo is not relevant, but I want to keep it as reference... https://github.com/lefty01/ESP32_TTGO_FTMS
