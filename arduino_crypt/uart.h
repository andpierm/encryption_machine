#pragma once
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sleep.h>

#define BAUD 19200
#define MYUBRR (F_CPU/16/BAUD-1)

extern volatile uint8_t len;
extern volatile uint8_t ignore;

uint8_t equals(uint8_t* a, uint8_t* b);

//! @brief initializes the uart peripheral
void UART_init(void);

//! @brief sends a char on the peripheral
void UART_putChar(uint8_t c);

//! @brief reads a char from the peripheral
uint8_t UART_getChar(void);

//! @brief high level function (exposed)
//!        reads a string until the first newline or 0
//!        returns the size read
uint8_t UART_getString(uint8_t* buf, uint8_t ignore_zero);

//! @brief high level function (exposed)
//!        writes a string until the first newline or 0
void UART_putString(uint8_t* buf);
