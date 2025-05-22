#include "crypt.h"
#include "uart.h"
#include <util/delay.h>

volatile uint8_t password[5] = {'1','2','3','4','5'};

int main(void) {
  UART_init();
  uint8_t cnt = 0;
  uint8_t n = 0;

  while(cnt < 3){
    uint8_t buf[255];
    n = UART_getString(buf, 0);
    if(equals(buf, (uint8_t*)password)) break;
    cnt++;
  }
  if(cnt == 3) return 1;
  UART_putString((uint8_t*)"OK\n");
  _delay_ms(10);
  while(1){
    uint8_t buf[255];
    n = UART_getString(buf, 0);
    if(equals(buf, (uint8_t*)"STOP")){
      break;
    }
    else if(n != 2 && (buf[0] != 'C' && buf[0] != 'D')){
      UART_putString((uint8_t*)"NOT A VALID OPTION\n");
    }
    else{
      if(buf[0] == 'C' && n == 2){
	while((n = UART_getString(buf, 1)) == 255){
	  crypt(buf, buf);
	  UART_putString(buf);
	  _delay_ms(50);
	}
        crypt(buf, buf);
        UART_putString(buf);
        _delay_ms(50);
      }
      else if(buf[0] == 'D' && n == 2){
        while((n = UART_getString(buf, 1)) == 255){
	  decrypt(buf, buf);
	  UART_putString(buf);
	  _delay_ms(50);
	}
	UART_putString(buf);
	_delay_ms(50);
      }
    }
    _delay_ms(50); // per permettere alla seriale di scrivere in tempo
  }
  UART_putString((uint8_t*)"FINE\n");
  _delay_ms(50);
  cli();
  return 0;
}
