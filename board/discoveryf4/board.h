#ifndef DISCOVERYF4_BOARD_H_
#define DISCOVERYF4_BOARD_H_

#include <platform/stm32f4/registers.h>
#include <platform/stm32f4/gpio.h>
#include <platform/stm32f4/usart.h>
#include <platform/stm32f4/nvic.h>
#include <platform/stm32f4/systick.h>

extern struct usart_dev console_uart;

#if defined(CONFIG_DBGPORT_USE_USART1)
