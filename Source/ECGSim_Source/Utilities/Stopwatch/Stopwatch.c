/*
 * Stopwatch.c
 *
 *  Created on: 06-Jan-2026
 *      Author: Mohammed Bin Saleem
 */

#include "Stopwatch.h"
#include <string.h>


stopwatch_t triggerSw;

bool createStopwatch(stopwatch_t* swInstance, TIM_HandleTypeDef* timer, uint32_t frequency, uint8_t lapCount)
{
	if(lapCount > MAX_NUMBER_OF_LAPS)
	{
		return false;
	}

	swInstance->state = false;
	swInstance->timerInstance = timer;
	swInstance->frequency = frequency;
	swInstance->totalLaps = lapCount;
	swInstance->currentValue = 0;
	swInstance->currentLap = 0;

	memset(swInstance->lapTimes,0.0,sizeof(swInstance->lapTimes));

//	__HAL_TIM_SET_COUNTER(swInstance->timerInstance,0);
	HAL_TIM_Base_Start(swInstance->timerInstance);

	return true;
}


bool lapStopwatch(stopwatch_t* swInstance)
{
	if(swInstance->state == false)
	{
		return false;
	}
	swInstance->state = false;
	uint32_t currentTime = __HAL_TIM_GET_COUNTER(swInstance->timerInstance);

	uint32_t delta_ticks = currentTime - swInstance->currentValue;


    swInstance->lapTimes[swInstance->currentLap] = (float)delta_ticks * 1000.0f / swInstance->frequency;

    swInstance->currentLap++;
    if(swInstance->currentLap >= swInstance->totalLaps)
    {
    	swInstance->currentLap = 0;
    }

    return true;

}

void startStopwatch(stopwatch_t* swInstance)
{
	if(swInstance->state == true)
	{
		return;
	}
	swInstance->state = true;
	swInstance->currentValue = __HAL_TIM_GET_COUNTER(swInstance->timerInstance);
}


bool getLapTime(stopwatch_t* swInstance, uint8_t lapNumber, float* lapTime)
{
	if(lapNumber >= swInstance->totalLaps)
	{
		return false;
	}

	*lapTime = swInstance->lapTimes[lapNumber];
	return true;
}

void getAverageLapTime(stopwatch_t* swInstance, uint32_t* averageLapTime)
{
	double lapSum = 0.0;
	float avgTime;
	for(int lapIndex = 0; lapIndex < swInstance->totalLaps; lapIndex++)
	{
		lapSum += swInstance->lapTimes[lapIndex];
	}

	avgTime = lapSum/(float)(swInstance->totalLaps);
	*averageLapTime = (uint32_t)avgTime;
}
