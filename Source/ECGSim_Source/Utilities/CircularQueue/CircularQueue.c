/**
 * @file CircularBuffer.c
 * @author Developer, Tismo
 * @date 10-Jan-2022
 * @brief This file provides methods to do circular queue management
 *
 * @attention COPYRIGHT(c) 2020 MIPM GMBH
 */

/**
 * @addtogroup UTILS
 * @{
 */

/**
 * @defgroup CircularQ Circular Queue
 * @brief Contains Circular Queue management APIs
 * @{
 */

#include "CircularQueue.h"

#include <stdlib.h>
#include <string.h>

static bool isCircularQueueFull(CircularQueue_t* queueHandle);
static bool isCircularQueueEmpty(CircularQueue_t* queueHandle);

bool CircularQueueInit_dynMem(CircularQueue_t* queueHandle, uint32_t maxSize)
{
	if(queueHandle == NULL || 0 >= maxSize)
	{
		return false;
	}

	queueHandle->maxSize = maxSize;
	queueHandle->headIndex = 0;
	queueHandle->tailIndex = 0;
	queueHandle->data = malloc(maxSize);

	if(queueHandle->data == NULL)
	{
		return false;
	}

	return true;
}
/**
 * @brief function to init queue with queueHandle params and checks if th eQueue handle passed is valid
 * @param queueHandle
 * @param pQBuffer
 * @param maxSize
 * @return true:success
 * false:failure
 * @note provide sufficiently large Buffers
 */
bool CircularQueueInit(CircularQueue_t* queueHandle, uint8_t* pQBuffer, uint32_t maxSize)
{
	if(queueHandle == NULL || NULL == pQBuffer || 0 >= maxSize )
	{
		return false;
	}

	queueHandle->maxSize = maxSize;
	queueHandle->headIndex = 0;
	queueHandle->tailIndex = 0;
	queueHandle->data = pQBuffer;

	return true;
}

/**
 * @brief function to de-initialize
 * @param queueHandle
 */
bool CircularQueueDeinit(CircularQueue_t* queueHandle)
{
	bool status = true;
	if(queueHandle != NULL)
	{
		//		free(queueHandle->data);
		queueHandle->maxSize = 0;
		queueHandle->headIndex = 0;
		queueHandle->tailIndex = 0;
		//		queueHandle->remainingDataSizeTobeRead = 0;
	}
	else status = false;
	return status;
}

/**
 * @brief function to de-initialize
 * @param queueHandle
 */
bool CircularQueueDeinit_dynMem(CircularQueue_t* queueHandle)
{
	bool status = true;
	if(queueHandle != NULL && queueHandle->data != NULL)
	{
		free(queueHandle->data);
		queueHandle->maxSize = 0;
		queueHandle->headIndex = 0;
		queueHandle->tailIndex = 0;
	}
	else status = false;
	return status;
}

/**
 * @brief function to reset Queue indices
 * @param queueHandle
 */
bool CircularQueueReset(CircularQueue_t* queueHandle)
{
	if(queueHandle == NULL)
	{
		return false;
	}

	queueHandle->headIndex = 0;
	queueHandle->tailIndex = 0;
	return true;
}

/**
 * @brief function to write a byte of data to the Queue
 * @param queueHandle
 * @param data
 * @return true:success
 * false:failure
 */
bool CircularQueueWriteByte(CircularQueue_t* queueHandle, uint8_t data)
{
	if(queueHandle == NULL)
	{
		return false;
	}

	if(isCircularQueueFull(queueHandle))
	{
		return false;
	}

	queueHandle->data[queueHandle->headIndex++] = data;
	queueHandle->headIndex = queueHandle->headIndex%queueHandle->maxSize;
	return true;
}

/**
 * @brief function to write Multiple bytes to the Queue
 * @param queueHandle
 * @param data
 * @param size
 * @return true:success
 * false:failure
 */
uint32_t CircularQueueWriteBytes(CircularQueue_t* queueHandle, uint8_t* data, uint32_t size)
{
	if(queueHandle == NULL)
	{
		return false;
	}

	uint32_t spaceAvailable = CircularQueueGetRemainingSpace(queueHandle);

	if(size > spaceAvailable)
	{
		size = spaceAvailable;
	}

	if(queueHandle->headIndex+size < queueHandle->maxSize)
	{
		memcpy(&queueHandle->data[queueHandle->headIndex], data, size);
		queueHandle->headIndex += size;
	}
	else
	{
		memcpy(&queueHandle->data[queueHandle->headIndex], &data[0], queueHandle->maxSize - queueHandle->headIndex);
		queueHandle->headIndex = ( queueHandle->headIndex + size ) % queueHandle->maxSize;
		memcpy(&queueHandle->data[0], &data[size-queueHandle->headIndex], queueHandle->headIndex);
	}
	//	queueHandle->remainingDataSizeTobeRead = CircularQueueGetRemainingData(queueHandle);

	return size;
}

/**
 * @brief function to read a byte from the Queue
 * @param queueHandle
 * @param data
 * @return true:success
 * false:failure
 */
bool CircularQueueReadByte(CircularQueue_t* queueHandle, uint8_t* data)
{
	if(queueHandle == NULL)
	{
		return false;
	}

	if(isCircularQueueEmpty(queueHandle))
	{
		return false;
	}

	*data = queueHandle->data[queueHandle->tailIndex++];
	queueHandle->tailIndex = queueHandle->tailIndex%queueHandle->maxSize;
	//	queueHandle->remainingDataSizeTobeRead = CircularQueueGetRemainingData(queueHandle);
	return true;
}

/**
 * @brief function to read multiple bytes from the Queue
 * @param queueHandle
 * @param data
 * @param size
 * @return true:success
 * false:failure
 */
uint32_t CircularQueueReadBytes(CircularQueue_t* queueHandle, uint8_t* data, uint32_t size)
{
	if(queueHandle == NULL)
	{
		return false;
	}

	uint32_t dataAvailable = CircularQueueGetRemainingData(queueHandle);

	if(size > dataAvailable)
	{
		size = dataAvailable;
	}

	if(queueHandle->tailIndex+size < queueHandle->maxSize)
	{
		memcpy(data, &queueHandle->data[queueHandle->tailIndex], size);
		queueHandle->tailIndex += size;
	}
	else
	{
		memcpy(&data[0], &queueHandle->data[queueHandle->tailIndex], queueHandle->maxSize - queueHandle->tailIndex);
		queueHandle->tailIndex = ( queueHandle->tailIndex + size ) % queueHandle->maxSize;
		memcpy(&data[size-queueHandle->tailIndex], &queueHandle->data[0], queueHandle->tailIndex);
	}
	//	queueHandle->remainingDataSizeTobeRead = CircularQueueGetRemainingData(queueHandle);

	return size;
}

/**
 * @brief function to get remaining space in the Queue
 * @param queueHandle
 * @return true:success
 * false:failure
 */
uint32_t CircularQueueGetRemainingSpace(CircularQueue_t* queueHandle)
{
	if(queueHandle == NULL)
	{
		return false;
	}

	if(queueHandle->tailIndex > queueHandle->headIndex)
	{
		return queueHandle->tailIndex - queueHandle->headIndex - 1;
	}
	else
	{
		//		return queueHandle->maxSize - queueHandle->tailIndex + queueHandle->headIndex - 1;
		return queueHandle->maxSize - queueHandle->headIndex + queueHandle->tailIndex;

	}
}


/**
 * @brief Function to get reminaing data in the Queue
 * @param queueHandle
 * @return true:success
 * false:failure
 */
uint32_t CircularQueueGetRemainingData(CircularQueue_t* queueHandle)
{
	if(queueHandle == NULL)
	{
		return false;
	}

	if(queueHandle->headIndex >= queueHandle->tailIndex)
	{
		return queueHandle->headIndex - queueHandle->tailIndex;
	}
	else
	{
		return queueHandle->maxSize - queueHandle->tailIndex + queueHandle->headIndex;
	}
}

/**@}*/ //Public Functions

/**
 * @defgroup CIRCQ_PVT_FUNC Circular Queue Private Functions
 * @{
 */
/**
 * @brief Private Function to check if the Queuue is Full
 * @param queueHandle Pointer to #CircularQueue_t
 * @return true : Full
 * false: not full
 */
bool isCircularQueueFull(CircularQueue_t* queueHandle)
{
	if(((queueHandle->headIndex+1)%queueHandle->maxSize) == queueHandle->tailIndex)
	{
		return true;
	}
	else
	{
		return false;
	}
}

/**
 * @brief Private function to check if Circular Queue is empty
 * @param Pointer to #CircularQueue_t
 * @return true : Full
 * false: not full
 */
bool isCircularQueueEmpty(CircularQueue_t* queueHandle)
{
	if(queueHandle->headIndex == queueHandle->tailIndex)
	{
		return true;
	}
	else
	{
		return false;
	}
}
/**@}*/ //Pvt functions
/**@}*/ //main defgroup Utils
/**@}*/ //add2grp utils
