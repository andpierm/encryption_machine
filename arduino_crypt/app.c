#include "crypt.h"
#include "uart.h"
#include <util/delay.h>

volatile uint8_t password[5] = {'1','2','3','4','5'};
uint8_t msg[256];

int main(void) {
  UART_init();
  uint8_t cnt = 0;
  while(cnt < 3){
    UART_getString(msg);
    UART_putString(msg);
    if(equals(msg, (uint8_t*)password)) break;
    cnt++;
  }
  if(cnt == 3) return 1;
  UART_putString((uint8_t*)"OK\n");
  while(1){
    uint8_t n = UART_getString(msg);
    if(equals(msg, (uint8_t*)"STOP")){
      break;
    }
    else if(n != 3 && (*msg != 'C' || *msg != 'D')){
      UART_putString((uint8_t*)"NOT A VALID OPTION\n");
    }
    else{
      if(*msg == 'C'){
	UART_getString(msg);
	crypt(msg);
	UART_putString(msg);
      }
      else{
        UART_getString(msg);
	decrypt(msg);
	UART_putString(msg);
      }
    }
    _delay_ms(50); // per permettere alla seriale di scrivere in tempo
  }
  UART_putString((uint8_t*)"FINE\n");
  _delay_ms(50);
  return 0;
}
