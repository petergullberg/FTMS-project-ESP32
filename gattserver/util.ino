//util.ino
#include <ctype.h>
#include "util.h"


/**
 * Convert ascii hex character to nibble, and 0x00 on error....
 * @param[in] hex ascii charcter '0' - 'f', either upper or lower case.
 * @param[out] hex-value (0x00-0x0F) 
 */ 
byte hexToNibble(byte x)
{
  //x = tolower(x);
  if ( (x >= '0') && (x <= '9'))
    return x - '0';
  if ((x >= 'a') && (x <= 'f'))
    return x - 'a' + 10;
  if ((x >= 'A') && (x <= 'F'))
    return x - 'A' + 10;
  return 0x00;
}

// hexstr2bytes moved to nvram for the moment!!!


/**
 * Convert a nibble to asci-value
 * @param[in] nibble a hexadecimal value from 0x00-0x0F
 * @param[out] The nibble converted, example '1' in lowercase
 */ 
char nibble2asciihex(byte nibble)
{
	return "0123456789abcdef"[nibble&0x0F];
}


/**
 * Convert a string of hexadecimal characters into a byte-buffer
 * @param[in] str string that contains hexavalues 
 * @param[in] dest_buf 
 * @param[in] len the length of the str buffer
 */ 
void hexStr2bytes(const char *str, byte *dest_buf, int len)
{
  if ( len & 0x01) {
    return;
  }
  for (int i = 0; i < len/2; i++) {
    dest_buf[i] = (hexToNibble(str[i*2]) << 4) | hexToNibble(str[(i*2)+ 1]);
  }
}
	
/**
 * Convert a byte-buffer of n to a hexadecimal string
 * @param[in] in_buf
 * @param[in] len the length of the buffer
 * @param[in] dest_str string that contains hexavalues
 */ 
void bytes2hexStr(byte *in_buf, int len, char *dest_str )
{
  for (int cnt = 0; cnt < len; cnt++) {
    dest_str[2*cnt]     = nibble2asciihex(in_buf[cnt]>>4);
    dest_str[(2*cnt)+1] = nibble2asciihex(in_buf[cnt]);
    dest_str[(2*cnt)+2] = 0x00;
  }
}
	
