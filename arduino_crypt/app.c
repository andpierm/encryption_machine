#include "crypt.h"
#include "uart.h"
#include <util/delay.h>

volatile uint8_t password[6] = {'1','2','3','4','5', '\0'};
volatile uint8_t len = 0;
volatile uint8_t ignore = 0;
uint8_t buf[MAX_BUF_LENGTH] = {0};

int main(void) {
  UART_init();
  uint8_t cnt = 0;
  uint8_t n = 0;

  while(cnt < 3){ // N.B: se vengono inserite stringhe > 255 caratteri allora lo conta cnt = cnt + 2
    n = UART_getString(buf, 0);
    if(equals(buf, (uint8_t*)password)) break;
    UART_putChar('N');
    UART_putChar(0);
    cnt++;
  }
  if(cnt == 3) return 1;
  UART_putString((uint8_t*)"OK", 2);
  UART_putChar(0);
  _delay_ms(10);
  while(1){
    n = UART_getString(buf, 0);
    if(equals(buf, (uint8_t*)"STOP")){
      break;
    }
    else if(n != 1 && (buf[0] != 'C' && buf[0] != 'D')){
      UART_putString((uint8_t*)"NOT A VALID OPTION", 18);
    }
    else{
      if(buf[0] == 'C' && n == 2){
	len = 0;
        n = UART_getString(buf, 1);
        _delay_ms(50); // per aspettare se l'utente ha inserito più byte del necessario di essere accumulati
	               // per farne poi la verifica
	ignore = 0;
	if(len > n) {
	  UART_putString((uint8_t*)"HAI INSERITO PIU BYTE DI QUELLI CHE AVEVI DETTO!", 48);
	  _delay_ms(50);
	  return 1;
	}
        crypt(buf, n);
        UART_putString(buf, n);
        _delay_ms(50);
      }
      else if(buf[0] == 'D' && n == 2){
	len = 0;
        n = UART_getString(buf, 1);
	_delay_ms(50);
	ignore = 0;
	if(len > n) {
	  UART_putString((uint8_t*)"HAI INSERITO PIU BYTE DI QUELLI CHE AVEVI DETTO!", 48);
	  _delay_ms(50);
	  return 1;
	}
	decrypt(buf, n);
	UART_putString(buf, n);
	_delay_ms(50);
      }
    }
    _delay_ms(50); // per permettere alla seriale di scrivere in tempo
  }
  // UART_putString((uint8_t*)"FINE\0");
  _delay_ms(50);
  cli();
  return 0;
}
