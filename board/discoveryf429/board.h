#ifndef DISCOVERYF429_BOARD_H_
#define DISCOVERYF429_BOARD_H_

#include <platform/stm32f429/registers.h>
#include <platform/stm32f429/gpio.h>
#include <platform/stm32f429/usart.h>
#include <platform/stm32f429/nvic.h>
#include <platform/stm32f429/systick.h>

extern struct usart_dev console_uart;

#if defined(CONFIG_DBGPORT_USE_USART1)

#define BOARD_UART_DEVICE	USART1_IRQn
#define BOARD_UART_HANDLER	USART1_HANDLER
#define BOARD_USART_FUNC	af_usart1
#define BOARD_USART_CONFIGS			\
	.base = USART1_BASE,			\
	.rcc_apbenr = RCC_USART1_APBENR,	\
	.rcc_reset = RCC_APB2RSTR_USART1RST,
#define BOARD_USART_TX_IO_PORT	GPIOA
#define BOARD_USART_TX_IO_PIN	9
#define BOARD_USART_RX_IO_PORT	GPIOA
#define BOARD_USART_RX_IO_PIN	10


#elif defined(CONFIG_DBGPORT_USE_USART2)

#define BOARD_UART_DEVICE	USART2_IRQn
#define BOARD_UART_HANDLER	USART2_HANDLER
#define BOARD_USART_FUNC	af_usart2
#define BOARD_USART_CONFIGS			\
	.base = USART2_BASE,			\
	.rcc_apbenr = RCC_USART2_APBENR,	\
	.rcc_reset = RCC_APB1RSTR_USART2RST,
#define BOARD_USART_TX_IO_PORT	GPIOD
#define BOARD_USART_TX_IO_PIN	5
#define BOARD_USART_RX_IO_PORT	GPIOD
#define BOARD_USART_RX_IO_PIN	6

#else	/* default: USART4 */
/* CONFIG_DBGPORT_USE_USART4 */

#define BOARD_UART_DEVICE	UART4_IRQn
#define BOARD_UART_HANDLER	UART4_HANDLER
#define BOARD_USART_FUNC	af_uart4
#define BOARD_USART_CONFIGS			\
	.base = UART4_BASE,			\
	.rcc_apbenr = RCC_UART4_APBENR,		\
	.rcc_reset = RCC_APB1RSTR_USART4RST,
#define BOARD_USART_TX_IO_PORT	GPIOC
#define BOARD_USART_TX_IO_PIN	11
#define BOARD_USART_RX_IO_PORT	GPIOC
#define BOARD_USART_RX_IO_PIN	10

#endif

#endif	/* DISCOVERYF429_BOARD_H_ */
