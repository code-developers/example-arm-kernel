#ifndef STM32P103_BOARD_H_
#define STM32P103_BOARD_H_

#include <platform/stm32f1/registers.h>
#include <platform/stm32f1/gpio.h>
#include <platform/stm32f1/usart.h>
#include <platform/stm32f1/nvic.h>
#include <platform/stm32f1/systick.h>

extern struct usart_dev console_uart;

#if defined(CONFIG_DBGPORT_USE_USART1)