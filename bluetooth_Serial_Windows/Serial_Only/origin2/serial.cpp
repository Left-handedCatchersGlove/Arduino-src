#include <avr/io.h>
#include <avr/interrupt.h>

/* 設定 */
#define SERIAL_SYS_CLOCK	20e6
#define SERIAL_BAUDRATE		9600
#define SERIAL_RX_BUFSIZE	64
#define SERIAL_TX_BUFSIZE	64

/* 割り込み制御マクロ */
#define SERIAL_TXIE_ENABLE()	UCSR0B |= _BV(UDRIE0)
#define SERIAL_TXIE_DISABLE()	UCSR0B &=~_BV(UDRIE0)
#define SERIAL_RXCIE_ENABLE()	UCSR0B |= _BV(RXCIE0)
#define SERIAL_RXCIE_DISABLE()	UCSR0B &=~_BV(RXCIE0)

/* 受信FIFO */
volatile struct {
	uint8_t buf[SERIAL_RX_BUFSIZE+1];
	uint8_t read;
	uint8_t write;
} serial_rx;

/* 送信FIFO */
volatile struct {
	uint8_t buf[SERIAL_TX_BUFSIZE+1];
	uint8_t read;
	uint8_t write;
} serial_tx;

/* シリアル通信の開始 */
void serial_init(uint16_t baud)
{
	// キューのリセット
	serial_rx.read = serial_rx.write = 0;
	serial_tx.read = serial_tx.write = 0;
	
	// USARTレジスタの設定
	UCSR0A = 0;
	UCSR0B = _BV(RXCIE0) | _BV(RXEN0) | _BV(TXEN0);
	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
	UBRR0 = (SERIAL_SYS_CLOCK / 16) / baud - 1;
	
	// 全割り込み許可
	sei();
}

/* シリアル通信の停止 */
void serial_end(void)
{
	UCSR0A = UCSR0B = UCSR0C = 0;
}

/* データを送信キューに追加する. 戻り値：追加できた数 */
uint8_t serial_send(uint8_t *buf, uint8_t size)
{
	uint8_t next, ret;
	
	// 排他制御のようなもの
	SERIAL_TXIE_DISABLE();
	
	ret = 0;
	next = (serial_tx.write + 1) % SERIAL_TX_BUFSIZE;
	while ( next != serial_tx.read && ret < size ) {
		serial_tx.buf[serial_tx.write] = buf[ret];
		serial_tx.write = next;
		next = (serial_tx.write + 1) % SERIAL_TX_BUFSIZE;
		ret++;
	}
	
	// 送信キューが空で無いときに割り込み再開
	if ( serial_tx.write != serial_tx.read ) {
		SERIAL_TXIE_ENABLE();
	}
	
	return ret;
}

/* 受信キューにあるデータを受け取る． 戻り値：受け取ったデータ数 */
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

/* データ受信割り込み */
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

/* データ送信可能割り込み */
ISR (USART0_UDRE_vect)
{
	UDR0 = serial_tx.buf[serial_tx.read];
	serial_tx.read = (serial_tx.read + 1) % SERIAL_TX_BUFSIZE;
	if ( serial_tx.read == serial_tx.write ) {
		SERIAL_TXIE_DISABLE();
	}
}