#include "crypt.h"
#include "uart.h"
#include <util/delay.h>

volatile uint8_t password[5] = {'1','2','3','4','5'};
volatile uint8_t index_rx = 0;
volatile uint8_t index_tx = 0;
uint8_t msg_rx[256]; 
uint8_t msg_tx[256];

int main(void) {
  UART_init();
  uint8_t cnt = 0;
  uint8_t n = 0;
  while(cnt < 3){
    n = UART_getString(msg_rx+index_rx);
    if(equals(msg_rx+index_rx, (uint8_t*)password)) break;
    index_rx = (index_rx+n) % sizeof(msg_rx);
    cnt++;
  }
  if(cnt == 3) return 1;
  UART_putString((uint8_t*)"OK\n");
  _delay_ms(10);
  while(1){
    n = UART_getString(msg_rx+index_rx);
    if(equals(msg_rx+index_rx, (uint8_t*)"STOP")){
      break;
    }
    else if(n != 3 && (*(msg_rx+index_rx) != 'C' && *(msg_rx+index_rx) != 'D')){
      UART_putString((uint8_t*)"NOT A VALID OPTION\n");
    }
    else{
      index_rx = (index_rx+n) % sizeof(msg_rx);
      if(*(msg_rx+index_rx) == 'C'){
	while((n = UART_getString(msg_rx + index_rx)) == 196){ // 196 e non 255 per non sovrascrivere dati non ancora letti
	  crypt(msg_rx + index_rx, msg_tx + index_tx);
	  index_rx = (index_rx+n) % sizeof(msg_rx);
	  //UART_putString(msg_tx + index_tx);
	  index_tx = (index_tx+n) % sizeof(msg_tx);
	  _delay_ms(50);
	}
        crypt(msg_rx + index_rx, msg_tx + index_tx);
	index_rx = (index_rx+n) % sizeof(msg_rx);
        //UART_putString(msg_tx + index_tx);
	index_tx = (index_tx+n) % sizeof(msg_tx);
        _delay_ms(50);
      }
      else{
        while((n = UART_getString(msg_rx + index_rx)) == 196){
	  decrypt(msg_rx + index_rx, msg_tx + index_tx);
	  index_rx = (index_rx+n) % sizeof(msg_rx);
	  UART_putString(msg_tx + index_tx);
	  index_tx = (index_tx+n) % sizeof(msg_tx);
	  _delay_ms(50);
	}
        index_rx = (index_rx+n) % sizeof(msg_rx);
	UART_putString(msg_tx + index_tx);
	index_tx = (index_tx+n) % sizeof(msg_tx);
	_delay_ms(50);
      }
    }
    _delay_ms(50); // per permettere alla seriale di scrivere in tempo
  }
  UART_putString((uint8_t*)"FINE\n");
  _delay_ms(50);
  return 0;
}
