#include "crypt.h"

void crypt(uint8_t *msg_rx, uint8_t *msg_tx){
  while(*msg_rx != '\n' && *msg_rx != '\0'){
    *msg_tx = *msg_rx;
    *msg_tx ^= SECRET_KEY1;
    *msg_tx ^= SECRET_KEY2;
    if(*msg_tx == '\0') *msg_tx = 1;
    msg_rx++;
    msg_tx++;
  }
}

void decrypt(uint8_t *msg_rx, uint8_t *msg_tx){
  while(*msg_rx != '\n' && *msg_rx != '\0'){
    *msg_tx = *msg_rx;
    if(*msg_tx == 1) *msg_tx = '\0';
    *msg_tx ^= SECRET_KEY2;
    *msg_tx ^= SECRET_KEY1;
    msg_rx++;
    msg_tx++;
  }
}
