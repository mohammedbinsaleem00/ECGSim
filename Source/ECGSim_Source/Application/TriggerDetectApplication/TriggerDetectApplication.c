/*
 * TriggerDetectApplication.c
 *
 *  Created on: 05-Jan-2026
 *      Author: Mohammed Bin Saleem
 */


#include "TriggerDetectApplication.h"
#include "Encoder/COBS/cobs.h"
#include "Stopwatch.h"
#include "CLIApplication.h"
#include "customUART.h"
#include "tim.h"


#include <string.h>
#include <stdio.h>
#define TRIGGER_END_VALUE 							0
#define TRIGGER_BUFFER_SIZE 						100


bool g_triggerReceived = false;
bool g_pauseTriggerDetect = false;

uint8_t g_decodedTriggerBuffer[TRIGGER_BUFFER_SIZE];

struct triggerBuffer{
	uint8_t buffer[TRIGGER_BUFFER_SIZE];
	uint32_t triggerIndex;
}g_triggerBuffer;




bool triggerDetectApplicationInit()
{
	if(createStopwatch(&triggerSw, &htim2, 1000, 5))
	{
//		startStopwatch(&triggerSw);
		return true;
	}

	return false;
}
bool getCharacter(char *receivedCharacter)
{
	return UART_ReadByteNonBlocking(TRIGGER_UART, (uint8_t *)receivedCharacter);
}


void resetTriggerBuffer()
{
	memset(g_triggerBuffer.buffer, 0, TRIGGER_BUFFER_SIZE);
	g_triggerBuffer.triggerIndex = 0;
}


void addToCommandBuffer(char a_data)
{
	if(g_triggerBuffer.triggerIndex >= TRIGGER_BUFFER_SIZE)
	{
		CLI_PrintLine("TOO MANY BYTES");
		resetTriggerBuffer();
		return ;
	}
	g_triggerBuffer.buffer[g_triggerBuffer.triggerIndex++] = a_data;
}

bool constructTriggerPayload()
{
	char receivedCharacter;
	if(getCharacter(&receivedCharacter))
	{
		addToCommandBuffer(receivedCharacter);
		if(receivedCharacter == TRIGGER_END_VALUE)
		{
			g_triggerReceived = true;
			return true;
		}
	}

	return false;
}


void extractTriggerData()
{
	cobs_decode_result decodeResult = cobs_decode(g_decodedTriggerBuffer, TRIGGER_BUFFER_SIZE, g_triggerBuffer.buffer, g_triggerBuffer.triggerIndex);

	resetTriggerBuffer();
	if(g_decodedTriggerBuffer[7] == 0xA5)
	{

		lapStopwatch(&triggerSw);
//		if(lapStopwatch(&triggerSw))
//		{
//			getAverageLapTime(&triggerSw, &avgLapTime_ms);
//			char str[30];
//			int len = sprintf(str,"Avg: %lu ms\n",avgLapTime_ms);
//			CLI_Print(str,len);
//		}


	}
}





void triggerProcess()
{
	if(g_pauseTriggerDetect)
		return;

	if(constructTriggerPayload() != true)
	{
		return;
	}

	extractTriggerData();

}

void pauseTriggerDetect()
{
	g_pauseTriggerDetect = true;
	resetTriggerBuffer();
}

void resumeTriggerDetect()
{
	g_pauseTriggerDetect = false;
}
