/*
 * pinConfig.h
 *
 *  Created on: 30-Dec-2025
 *      Author: Mohammed Bin Saleem
 */

#ifndef PINCONFIG_H_
#define PINCONFIG_H_

#include "stm32f1xx.h"
#include "stm32f1xx_hal.h"


#define DEBUG_UART_INSTANCE				USART1
#define DEBUG_UART_CLK_ENABLE()			__HAL_RCC_USART1_CLK_ENABLE()
#define DEBUG_UART_CLK_DISABLE()		__HAL_RCC_USART1_CLK_DISABLE()
#define DEBUG_UART_PORT_CLK_ENABLE()	__HAL_RCC_GPIOA_CLK_ENABLE();

#define DEBUG_UART_TX_PIN 		GPIO_PIN_9
#define DEBUG_UART_TX_PORT		GPIOA

#define DEBUG_UART_RX_PIN 		GPIO_PIN_5
#define DEBUG_UART_RX_PORT		GPIOC
#endif /* PINCONFIG_H_ */
