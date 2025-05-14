#pragma once

#include <stdint.h>

#define SECRET_KEY 0x2F

void crypt(uint8_t *msg);
void decrypt(uint8_t *msg);


