#include "uart.h"

volatile uint8_t rx_data[MAX_BUF_LENGTH] = {0};
volatile uint8_t index_rx_isr = 0;
volatile uint8_t index_rx_read = 0;

volatile uint8_t tx_data[MAX_BUF_LENGTH] = {0};
volatile uint8_t index_tx_isr = 0;
volatile uint8_t index_tx_put = 0;

uint8_t equals(uint8_t* a, uint8_t* b){
  while(*a != '\0' || *b != '\0'){
    if(*a != *b) return 0;
    a++;
    b++;
  }
  return 1;
}

void UART_init(void) {
  // Set baud rate
  UBRR0H = (uint8_t)(MYUBRR>>8);
  UBRR0L = (uint8_t)MYUBRR;

  UCSR0C = (1<<UCSZ01) | (1<<UCSZ00); // 8-bit data - Character Size
  UCSR0B = (1<<RXEN0) | (1<<RXCIE0) | (1<<TXEN0); // Enable interrupt and even for complete transmission
  sei();
}

ISR(USART_UDRE_vect){
  if(index_tx_isr != index_tx_put){
    UDR0 = tx_data[index_tx_isr];
    index_tx_isr = (index_tx_isr + 1) % sizeof(tx_data);
  }
  else UCSR0B &= ~(1<<UDRIE0); // se non c'è nulla da scrivere disabilita interrupt
}

void UART_putChar(uint8_t c){
  tx_data[index_tx_put] = c;
  index_tx_put = (index_tx_put + 1) % sizeof(tx_data);
  UCSR0B |= (1<<UDRIE0); // abilita interrupt per buffer pronto ad essere trasmesso
}

ISR(USART_RX_vect){
  uint8_t next = (index_rx_isr + 1) % sizeof(rx_data);
  if(ignore) len++;
  if(next != index_rx_read){ // se c'è spazio e se non supera la lunghezza consentita
    rx_data[index_rx_isr] = UDR0;
    index_rx_isr = next;
  }
}

uint8_t UART_getChar(void){
  cli(); // l'isr potrebbe essere eseguita quando faccio getchar
  if(index_rx_read == index_rx_isr){
    sei();
    return 0;
  }

  uint8_t c = rx_data[index_rx_read];
  index_rx_read = (index_rx_read + 1) % sizeof(rx_data);
  sei();
  return c;
}

uint8_t UART_getString(uint8_t* buf, uint8_t ignore_zero){
  uint8_t* b0=buf;
  uint8_t len_internal = MAX_BUF_LENGTH;
  if(ignore_zero){
    while(index_rx_isr == index_rx_read) sleep_cpu();
    len_internal = UART_getChar();
    UART_putChar('O'); // ACK per ok manda i dati
    _delay_ms(50);
  }
  ignore = ignore_zero;
  while(buf-b0 < len_internal){
    while(index_rx_isr == index_rx_read) sleep_cpu();
    
    uint8_t c=UART_getChar();
    *buf=c;
    ++buf;
    if(!ignore_zero && !c){
      *buf=0;
      return buf-b0;
    }
  }
  *buf = 0;
  return len_internal;
}

void UART_putString(uint8_t* buf, uint8_t n){
  if(n > 255) return;
  uint8_t *b0 = buf;
  while(buf-b0 < n){
    UART_putChar(*buf);
    ++buf;
  }
}
