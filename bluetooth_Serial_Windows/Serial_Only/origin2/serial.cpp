#include <avr/io.h>
#include <avr/interrupt.h>

/* �ݒ� */
#define SERIAL_SYS_CLOCK	20e6
#define SERIAL_BAUDRATE		9600
#define SERIAL_RX_BUFSIZE	64
#define SERIAL_TX_BUFSIZE	64

/* ���荞�ݐ���}�N�� */
#define SERIAL_TXIE_ENABLE()	UCSR0B |= _BV(UDRIE0)
#define SERIAL_TXIE_DISABLE()	UCSR0B &=~_BV(UDRIE0)
#define SERIAL_RXCIE_ENABLE()	UCSR0B |= _BV(RXCIE0)
#define SERIAL_RXCIE_DISABLE()	UCSR0B &=~_BV(RXCIE0)

/* ��MFIFO */
volatile struct {
	uint8_t buf[SERIAL_RX_BUFSIZE+1];
	uint8_t read;
	uint8_t write;
} serial_rx;

/* ���MFIFO */
volatile struct {
	uint8_t buf[SERIAL_TX_BUFSIZE+1];
	uint8_t read;
	uint8_t write;
} serial_tx;

/* �V���A���ʐM�̊J�n */
void serial_init(uint16_t baud)
{
	// �L���[�̃��Z�b�g
	serial_rx.read = serial_rx.write = 0;
	serial_tx.read = serial_tx.write = 0;
	
	// USART���W�X�^�̐ݒ�
	UCSR0A = 0;
	UCSR0B = _BV(RXCIE0) | _BV(RXEN0) | _BV(TXEN0);
	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
	UBRR0 = (SERIAL_SYS_CLOCK / 16) / baud - 1;
	
	// �S���荞�݋���
	sei();
}

/* �V���A���ʐM�̒�~ */
void serial_end(void)
{
	UCSR0A = UCSR0B = UCSR0C = 0;
}

/* �f�[�^�𑗐M�L���[�ɒǉ�����. �߂�l�F�ǉ��ł����� */
uint8_t serial_send(uint8_t *buf, uint8_t size)
{
	uint8_t next, ret;
	
	// �r������̂悤�Ȃ���
	SERIAL_TXIE_DISABLE();
	
	ret = 0;
	next = (serial_tx.write + 1) % SERIAL_TX_BUFSIZE;
	while ( next != serial_tx.read && ret < size ) {
		serial_tx.buf[serial_tx.write] = buf[ret];
		serial_tx.write = next;
		next = (serial_tx.write + 1) % SERIAL_TX_BUFSIZE;
		ret++;
	}
	
	// ���M�L���[����Ŗ����Ƃ��Ɋ��荞�ݍĊJ
	if ( serial_tx.write != serial_tx.read ) {
		SERIAL_TXIE_ENABLE();
	}
	
	return ret;
}

/* ��M�L���[�ɂ���f�[�^���󂯎��D �߂�l�F�󂯎�����f�[�^�� */
uint8_t serial_recv(uint8_t *buf, uint8_t size)
{
	uint8_t ret = 0;
	
	SERIAL_RXCIE_DISABLE();
	while ( serial_rx.read != serial_rx.write && ret < size ) {
		buf[ret] = serial_rx.buf[serial_rx.read];
		serial_rx.read = (serial_rx.read + 1) % SERIAL_RX_BUFSIZE;
		ret++;
	}
	SERIAL_RXCIE_ENABLE();
	
	return ret;
}

/* �f�[�^��M���荞�� */
ISR (USART0_RX_vect)
{
	uint8_t next = (serial_rx.write + 1) % SERIAL_RX_BUFSIZE;
	if ( next == serial_rx.read ) {
		next = UDR0;
		return;
	}
	serial_rx.buf[serial_rx.read] = UDR0;
	serial_rx.write = next;
}

/* �f�[�^���M�\���荞�� */
ISR (USART0_UDRE_vect)
{
	UDR0 = serial_tx.buf[serial_tx.read];
	serial_tx.read = (serial_tx.read + 1) % SERIAL_TX_BUFSIZE;
	if ( serial_tx.read == serial_tx.write ) {
		SERIAL_TXIE_DISABLE();
	}
}