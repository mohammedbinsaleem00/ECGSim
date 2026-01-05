/*
 * customUART.h
 *
 *  Created on: 30-Dec-2025
 *      Author: Mohammed Bin Saleem
 */

#ifndef CUSTOMHAL_CUSTOMUART_CUSTOMUART_H_
#define CUSTOMHAL_CUSTOMUART_CUSTOMUART_H_


#include <stdint.h>
#include <stdbool.h>

typedef enum{
	DEBUG_UART, //UART1
	TRIGGER_UART, //UART2
	UART_COUNT
}UARTType_t;

#define DEBUG_UART_TX_QUEUE_SIZE 1024
#define DEBUG_UART_RX_QUEUE_SIZE 1024

#define TRIGGER_UART_TX_QUEUE_SIZE 1024
#define TRIGGER_UART_RX_QUEUE_SIZE 1024


bool UART_Init(UARTType_t uartType);


bool UART_Write(UARTType_t uartType, uint8_t *pdata, uint32_t size);


bool UART_ReadByteNonBlocking(UARTType_t uart, uint8_t *pdata);

bool UART_ClearRxBuffer(UARTType_t uartType);

bool UART_ClearTxBuffer(UARTType_t uartType);
#endif /* CUSTOMHAL_CUSTOMUART_CUSTOMUART_H_ */
