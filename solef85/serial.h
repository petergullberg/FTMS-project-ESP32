#ifndef SERIAL_H_
#define SERIAL_H_
uint16_t _serial_wait_cmd(uint8_t *buf, uint16_t buf_size, uint16_t *len, uint16_t timeout);
uint16_t serial_get_cmd( uint8_t *buf, uint16_t buf_size, uint16_t *len);
int serial_poll(uint16_t timeout);
int serial_is_cmd(void);
void serial_init(void);


#endif //SERIAL_H_