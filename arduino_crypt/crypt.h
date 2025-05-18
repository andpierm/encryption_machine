#pragma once

#include <stdint.h>

#define SECRET_KEY1 0x2F
#define SECRET_KEY2 0xAE

void crypt(uint8_t *msg);
void decrypt(uint8_t *msg);


