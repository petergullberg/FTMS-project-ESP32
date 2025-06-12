#include "serial.h"
// Simple searial wait cmd, with a timeout

#define my_millis millis
#define CMD_MAX_LEN 127
#define CMD_TIME_OUT 100    // intercharacter timeout..

static uint8_t serBuf[CMD_MAX_LEN+1];
static uint16_t serBufLen;
static volatile bool fSerial;

uint16_t _serial_wait_cmd(uint8_t *buf, uint16_t buf_size, uint16_t *len, uint16_t timeout)
{
  uint32_t last_ch_time;
  uint8_t ch;

  *len = 0;
  last_ch_time = my_millis();
  // Assume that we lost communication if timeout>100ms;
  while (1) {
    if ( my_millis() > (last_ch_time + timeout) ) {
      buf[*len] = 0x00;    // zero filled string
      return *len;
    }
    if (Serial.available() > 0) {
      last_ch_time = my_millis();    // reset byte timer
      ch = Serial.read();
      switch ( ch ) {
        //case ' ':
        case '\r':
          break;
        case 0x08:  // DEL
          if ( *len ) {
            buf[*len] = 0x00;
            (*len)--;
            buf[*len] = 0x00;
            //Serial.printf("%c", ch);
          }
          break;
        case '\n':
          //Serial.printf(":END\n");
          buf[*len] = 0x00;    // zero filled string
          return *len;
        default:
          // ECHO The character
          //Serial.printf("%c", ch);
          buf[(*len)++] = ch;  // Ugly wiht prescedence
          if (*len >= buf_size) {
            Serial.printf("TOO LONG %d\n", (int)*len);
            buf[*len] = 0x00;    // zero filled string
            return *len;
          }
          break;
      }
    }
  }
  Serial.printf("ERROR\n");
  // Never reach here!
}

uint16_t serial_get_cmd( uint8_t *buf, uint16_t buf_size, uint16_t *len)
{
  if ( fSerial && (serBufLen < buf_size) )  {
    memcpy( buf, serBuf, serBufLen); 
    *len = serBufLen;
  }
  else {
    *len=0;
  }

  fSerial=false;    // discard, for all reasons
  return *len;
}
int serial_is_cmd(void)
{
  return fSerial;

}

int serial_poll(uint16_t timeout)
{
  if ( false == fSerial) {
    if ( _serial_wait_cmd(serBuf, sizeof(serBuf)-1, &serBufLen, timeout) ) {
      fSerial=true;
    }
  }
  else {} // do nothing
  return fSerial;
}

void serial_init(void)
{
  fSerial = 0;
  serBufLen=0;
}
