/*
 * customUART.c
 *
 *  Created on: 30-Dec-2025
 *      Author: Mohammed Bin Saleem
 */
#include "customUART.h"
#include "CircularQueue/CircularQueue.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "usart.h"
#include "pinConfig.h"
#include "CommonConfigurations.h"
#define UART_IT_WRITE_CHUNK_SIZE 251

QueueHandle_t g_DebugUARTTxQueue;
QueueHandle_t g_DebugUARTRxQueue;
QueueHandle_t g_TriggerUARTTxQueue;
QueueHandle_t g_TriggerUARTRxQueue;

UART_HandleTypeDef* g_uartHandler[UART_COUNT];

uint8_t g_TxByte[UART_COUNT][UART_IT_WRITE_CHUNK_SIZE];
uint8_t g_RxByte[UART_COUNT];



bool initDebugUart()
{


		MX_USART1_UART_Init();
		g_uartHandler[DEBUG_UART] = &huart1;
		g_DebugUARTTxQueue = xQueueCreate(DEBUG_UART_TX_QUEUE_SIZE,1);
		g_DebugUARTRxQueue = xQueueCreate(DEBUG_UART_RX_QUEUE_SIZE,1);

		if( NULL == g_DebugUARTTxQueue || NULL == g_DebugUARTRxQueue)
		{
			return false;

		}

		if(HAL_UART_Receive_IT(g_uartHandler[DEBUG_UART], &g_RxByte[DEBUG_UART], 1) != HAL_OK)
		{
			return false;
		}
		return true;
}

bool initTriggerUart()
{


		MX_USART2_UART_Init();
		g_uartHandler[TRIGGER_UART] = &huart2;
		g_TriggerUARTTxQueue = xQueueCreate(TRIGGER_UART_TX_QUEUE_SIZE,1);
		g_TriggerUARTRxQueue = xQueueCreate(TRIGGER_UART_RX_QUEUE_SIZE,1);

		if( NULL == g_TriggerUARTTxQueue || NULL == g_TriggerUARTRxQueue)
		{
			return false;

		}

		if(HAL_UART_Receive_IT(g_uartHandler[TRIGGER_UART], &g_RxByte[TRIGGER_UART], 1) != HAL_OK)
		{
			return false;
		}
		return true;
}

bool UART_Init(UARTType_t uartType)
{
	bool status = true;
	if(uartType == DEBUG_UART)
	{
		initDebugUart();

	}
	else if(uartType == TRIGGER_UART)
	{
		initTriggerUart();

	}
	else
	{
		status = false;
	}
	return status;
}


bool UART_Write(UARTType_t uartType, uint8_t *pdata, uint32_t size)
{
	bool status = true;
	if( IS_VALID_PNTR(pdata) && 0 < size)
	{

		if(uartType == DEBUG_UART)
		{
			while(size --)
			{
				xQueueSendToBack(g_DebugUARTTxQueue, pdata,(TickType_t) 0);
				pdata++;
			}
		}
		else if (uartType == TRIGGER_UART)
		{
			while(size --)
			{
				xQueueSendToBack(g_TriggerUARTTxQueue, pdata,(TickType_t) 0);
				pdata++;
			}

		}


		if(g_uartHandler[uartType]->gState != HAL_UART_STATE_BUSY_TX)
		{
			__HAL_UART_ENABLE_IT(g_uartHandler[uartType], UART_IT_TC);
		}
	}
	else status = false;

	return status;


}


bool UART_ReadByteNonBlocking(UARTType_t uartType, uint8_t *pdata)
{
	bool status = false;
	if(IS_VALID_PNTR(pdata))
	{

		if(uartType == DEBUG_UART)
		{
			if(xQueueReceive(g_DebugUARTRxQueue, pdata, 0) == pdTRUE)
			{
				status = true;
			}
		}
		else if(uartType == TRIGGER_UART)
		{
			if(xQueueReceive(g_TriggerUARTRxQueue, pdata, 0) == pdTRUE)
			{
				status = true;
			}
		}


	}
	return status;
}


bool UART_ClearRxBuffer(UARTType_t uartType)
{
	bool status = false;


		if(uartType == DEBUG_UART)
		{
			xQueueReset(g_DebugUARTRxQueue);

		}
		else if(uartType == TRIGGER_UART)
		{
			xQueueReset(g_TriggerUARTRxQueue);

		}

		status = true;



	return status;

}

bool UART_ClearTxBuffer(UARTType_t uartType)
{
	bool status = false;


		if(uartType == DEBUG_UART)
		{
			xQueueReset(g_DebugUARTTxQueue);
		}
		else if(uartType == TRIGGER_UART)
		{
			xQueueReset(g_TriggerUARTTxQueue);
		}

		status = true;



	return status;

}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == g_uartHandler[DEBUG_UART]->Instance)
	{
		if(xQueueReceiveFromISR(g_DebugUARTTxQueue, &g_TxByte[DEBUG_UART], 0) == pdTRUE)
		{
			HAL_UART_Transmit_IT(g_uartHandler[DEBUG_UART], g_TxByte[DEBUG_UART], 1);
		}
	}
	else if(huart->Instance == g_uartHandler[TRIGGER_UART]->Instance)
	{
		if(xQueueReceiveFromISR(g_TriggerUARTTxQueue, &g_TxByte[TRIGGER_UART], 0) == pdTRUE)
		{
			HAL_UART_Transmit_IT(g_uartHandler[TRIGGER_UART], g_TxByte[TRIGGER_UART], 1);
		}
	}

}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)

{
	if(huart->Instance == g_uartHandler[DEBUG_UART]->Instance)
	{
		BaseType_t pxHigherPriorityTaskWoken;
		xQueueSendToBackFromISR(g_DebugUARTRxQueue, &g_RxByte[DEBUG_UART], &pxHigherPriorityTaskWoken);
		if(pxHigherPriorityTaskWoken)
		{
			taskYIELD();
		}

		HAL_UART_Receive_IT(g_uartHandler[DEBUG_UART], &g_RxByte[DEBUG_UART], 1);
	}

	else if(huart->Instance == g_uartHandler[TRIGGER_UART]->Instance)
	{
		BaseType_t pxHigherPriorityTaskWoken;
		xQueueSendToBackFromISR(g_TriggerUARTRxQueue, &g_RxByte[TRIGGER_UART], &pxHigherPriorityTaskWoken);
		if(pxHigherPriorityTaskWoken)
		{
			taskYIELD();
		}

		HAL_UART_Receive_IT(g_uartHandler[TRIGGER_UART], &g_RxByte[TRIGGER_UART], 1);
	}
}

