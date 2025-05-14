#include "crypt.h"

void crypt(uint8_t *msg){
  while(*msg != '\n'){
    *msg = ((*msg ^ SECRET_KEY)%95) + 32;
    
    msg++;
  }
}

void decrypt(uint8_t *msg){
  while(*msg != '\n'){
    *msg = (*msg-32 + 95) % 95;
    *msg ^= SECRET_KEY;
    msg++;
  }
}
