#ifndef UTIL_H_
#define UTIL_H_
byte hexToNibble(byte x);
char nibble2asciihex(byte nibble);
void hexStr2bytes(const char *str, byte *dest_buf, int len);
void bytes2hexStr(byte *in_buf, int len, char *dest_str );

#endif // ENDIF UTIL_H_