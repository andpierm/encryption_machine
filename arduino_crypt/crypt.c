#include "crypt.h"

void crypt(uint8_t *buf, uint8_t n){
  uint8_t cnt = n;
  while(cnt>0){
    *buf ^= SECRET_KEY1;
    *buf ^= SECRET_KEY2;
    buf++;
    cnt--;
  }
}

void decrypt(uint8_t *buf, uint8_t n){
  uint8_t cnt = n;
  while(cnt>0){
    *buf ^= SECRET_KEY2;
    *buf ^= SECRET_KEY1;
    buf++;
    cnt--;
  }
}
