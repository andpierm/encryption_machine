#include "uart.h"

volatile uint8_t data[256] = {0};
volatile uint8_t index_rx_isr = 0;
volatile uint8_t index_rx_read = 0;

volatile uint8_t index_tx_isr = 0;
volatile uint8_t index_tx_put = 0;

uint8_t equals(uint8_t* a, uint8_t* b){
  while(*a != '\n'){
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
    UDR0 = data[index_tx_isr];
    index_tx_isr = (index_tx_isr + 1) % sizeof(data);
  }
  else UCSR0B &= ~(1<<UDRIE0); // se non c'è nulla da scrivere disabilita interrupt
}

void UART_putChar(uint8_t c){
  data[index_tx_put] = c;
  index_tx_put = (index_tx_put + 1) % sizeof(data);
  UCSR0B |= (1<<UDRIE0); // abilita interrupt per buffer pronto ad essere trasmesso
}

ISR(USART_RX_vect){
  data[index_rx_isr] = UDR0;
  index_rx_isr = (index_rx_isr + 1) % sizeof(data);
}

uint8_t UART_getChar(void){
  if(index_rx_read == index_rx_isr) return 0;
  
  uint8_t c = data[index_rx_read];
  index_rx_read = (index_rx_read + 1) % sizeof(data);
  
  return c;
}

uint8_t UART_getString(uint8_t* buf){
  uint8_t* b0=buf;
  while(1){
    while(index_rx_isr == index_rx_read) sleep_cpu();
    
    uint8_t c=UART_getChar();
    *buf=c;
    ++buf;
    if (c==0)
      return buf-b0;
    
    if(c=='\n'||c=='\r'){
      *buf=0;
      ++buf;
      c = buf-b0;
      buf = b0;
      return c;
    }
  }
}

void UART_putString(uint8_t* buf){
  while(*buf){
    UART_putChar(*buf);
    ++buf;
  }
}
