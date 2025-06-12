#ifndef BLE_H_
#define BLE_H_
void _ble_send_command(uint8_t *cmd_buf, uint16_t len);
int ble_is_connected(void);
void _ble_send_command(uint8_t *cmd_buf, uint16_t len);


void ble_setup();
void ble_loop();


#endif // BLE_H_
