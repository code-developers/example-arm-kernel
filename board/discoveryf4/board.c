// includes
#include <platform/stm32f4/gpio.h>
#include <platform/stm32f4/usart.h>
#include "board.h"

struct usart_dev console_uart = {
	.u_num = 4,
	.baud = 115200,
	BOARD_USART_CONFIGS
	.tx = {
		.port = BOARD_USART_TX_IO_PORT,
		.pin = BOARD_USART_TX_IO_PIN,
		.pupd = GPIO_PUPDR_NONE,
		.type = GPIO_MODER_ALT,
		.func = BOARD_USART_FUNC,
		.o_type = GPIO_OTYPER_PP,
		.speed = GPIO_OSPEEDR_50M,
	},
	.rx = {
		.port = BOARD_USART_RX_IO_PORT,
		.pin = BOARD_USART_RX_IO_PIN,
		.pupd = GPIO_PUPDR_NONE,
		.type = GPIO_MODER_ALT,
		.func = BOARD_USART_FUNC,
		.o_type = GPIO_OTYPER_PP,
		.speed = GPIO_OSPEEDR_50M,
	},
};