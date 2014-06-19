#ifndef _SERIAL_H_
#define _SERIAL_H_

void serial_init(uint16_t baud);
void serial_end(void);
uint8_t serial_send(uint8_t *buf, uint8_t size);
uint8_t serial_recv(uint8_t *buf, uint8_t size);

#endif /* _SERIAL_H_ */