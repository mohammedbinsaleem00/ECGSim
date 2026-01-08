/*
 * Stopwatch.h
 *
 *  Created on: 06-Jan-2026
 *      Author: Mohammed Bin Saleem
 */

#ifndef UTILITIES_STOPWATCH_STOPWATCH_H_
#define UTILITIES_STOPWATCH_STOPWATCH_H_


#include <stdint.h>
#include <stdbool.h>

#include "stm32f1xx.h"
#include "stm32f1xx_hal.h"


#define MAX_NUMBER_OF_LAPS				10

typedef struct{
	bool state;
	TIM_HandleTypeDef* timerInstance;
	uint32_t frequency;
	uint32_t currentValue;
	float lapTimes[MAX_NUMBER_OF_LAPS];
	uint8_t currentLap;
	uint8_t totalLaps;
}stopwatch_t;

extern stopwatch_t triggerSw;

bool createStopwatch(stopwatch_t* swInstance, TIM_HandleTypeDef* timer, uint32_t frequency, uint8_t lapCount);
bool lapStopwatch(stopwatch_t* swInstance);
void startStopwatch(stopwatch_t* swInstance);
bool getLapTime(stopwatch_t* swInstance, uint8_t lapNumber, float* lapTime);
void getAverageLapTime(stopwatch_t* swInstance, uint32_t* averageLapTime);

#endif /* UTILITIES_STOPWATCH_STOPWATCH_H_ */
