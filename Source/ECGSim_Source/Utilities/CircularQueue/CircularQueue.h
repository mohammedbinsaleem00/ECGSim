/**
 * @file CircularBuffer.h
 * @author Developer, Tismo
 * @date 10-Jan-2022
 * @brief This file provides declarations for methods to do circular queue management
 *
 * @attention COPYRIGHT(c) 2020 MIPM GMBH
 */

#ifndef CIRCULARQUEUE_CIRCULARQUEUE_H_
#define CIRCULARQUEUE_CIRCULARQUEUE_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


/**
 * Data structure containing Queue params
 */
typedef struct{
	uint8_t* data;
	uint32_t maxSize;
	uint32_t headIndex;
	uint32_t tailIndex;
//	uint32_t remainingDataSizeTobeRead;
}CircularQueue_t;


/**
 * function to init queue with queueHandle params and checks if th eQueue handle passed is valid
 * @param queueHandle
 * @param pQBuffer
 * @param maxSize
 * @return true:success
 * false:failure
 */
bool CircularQueueInit(CircularQueue_t* queueHandle, uint8_t* pQBuffer, uint32_t maxSize);


bool CircularQueueInit_dynMem(CircularQueue_t* queueHandle, uint32_t maxSize);

/**
 * function to de-initialize
 * @param queueHandle
 */
bool CircularQueueDeinit(CircularQueue_t* queueHandle);

bool CircularQueueDeinit_dynMem(CircularQueue_t* queueHandle);

/**
 * function to reset Queue indices
 * @param queueHandle
 */
bool CircularQueueReset(CircularQueue_t* queueHandle);

/**
 * function to write a byte of data to the Queue
 * @param queueHandle
 * @param data
 * @return true:success
 * false:failure
 */
bool CircularQueueWriteByte(CircularQueue_t* queueHandle, uint8_t data);

/**
 * function to write Multiple bytes to the Queue
 * @param queueHandle
 * @param data
 * @param size
 * @return true:success
 * false:failure
 */
uint32_t CircularQueueWriteBytes(CircularQueue_t* queueHandle, uint8_t* data, uint32_t size);

/**
 * function to read a byte from the Queue
 * @param queueHandle
 * @param data
 * @return true:success
 * false:failure
 */
bool CircularQueueReadByte(CircularQueue_t* queueHandle, uint8_t* data);


/**
 * function to read multiple bytes from the Queue
 * @param queueHandle
 * @param data
 * @param size
 * @return true:success
 * false:failure
 */
uint32_t CircularQueueReadBytes(CircularQueue_t* queueHandle, uint8_t* data, uint32_t size);


/**
 * function to get remaining space in the Queue
 * @param queueHandle
 * @return true:success
 * false:failure
 */
uint32_t CircularQueueGetRemainingSpace(CircularQueue_t* queueHandle);


/**
 * Function to get reminaing data in the Queue
 * @param queueHandle
 * @return true:success
 * false:failure
 */
uint32_t CircularQueueGetRemainingData(CircularQueue_t* queueHandle);

#endif /* CIRCULARQUEUE_CIRCULARQUEUE_H_ */
