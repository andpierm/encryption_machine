#pragma once

#include <stdint.h>

#define SECRET_KEY 0xAF

void crypt(uint8_t *msg);
void decrypt(uint8_t *msg);


