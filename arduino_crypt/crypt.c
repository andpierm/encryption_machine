#include "crypt.h"

void crypt(uint8_t *msg){
  while(*msg != '\n'){
    *msg ^= SECRET_KEY1;
    *msg ^= SECRET_KEY2;
    msg++;
  }
}

void decrypt(uint8_t *msg){
  while(*msg != '\n'){
    *msg ^= SECRET_KEY2;
    *msg ^= SECRET_KEY1;
    msg++;
  }
}
