#include "crypt.h"
#include "uart.h"

volatile uint8_t password[5] = {'1','2','3','4','5'};
uint8_t msg[256];

int check_pass(uint8_t* password_to_check, volatile uint8_t* password){
  while(*password_to_check != '\n'){
    if(*password_to_check != *password){
      return 1;
    }
    password_to_check++;
    password++;
  }
   UART_putString((uint8_t*)"AOOOOO");
  return 0;
}

int main(void) {
  UART_init();
  uint8_t cnt = 0;
  while(cnt < 3){
    UART_getString(msg);
    if(!check_pass(msg, password))  UART_putString((uint8_t*)"OK");
    cnt++;
  }
  if(cnt == 3) return 1;
  UART_putString((uint8_t*)"OK");
  while(1){
    uint8_t n = UART_getString(msg);
    if(n != 1 && (*msg != (uint8_t)'C' || *msg != (uint8_t)'D')){
      UART_putString((uint8_t*)"NOT A VALID OPTION\n");
    }else{
      if(*msg == (uint8_t)'C'){
	// SIMULO CRIPTAZIONE
	UART_putString((uint8_t*)"STRINGA CRIPTATA\n");
      }
      else{
	// SIMULO DECRIPTAZIONE
	UART_putString((uint8_t*)"STRINGA DECRIPTATA\n");
      }
    }
  }
}
