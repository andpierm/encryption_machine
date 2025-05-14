#include "crypt.h"

void crypt(uint8_t *msg){
  while(*msg != '\n'){
    *msg ^= SECRET_KEY;
    msg++;
  }
}

void decrypt(uint8_t *msg){
  while(*msg != '\n'){
    *msg ^= SECRET_KEY;
    msg++;
  }
}
