#include <avr/io.h>
#include <util/delay.h>
#include "serial.h"

int main (void)
{
	uint8_t buf[16], n;
	
	serial_init(9600);
	while (1) {
		n = serial_recv(buf,sizeof(buf));
		if (n)	serial_send(buf,n);
		
		_delay_ms(1);	// ‰¼‘z•‰‰×
	}
	serial_end();
	
	return 0;
}