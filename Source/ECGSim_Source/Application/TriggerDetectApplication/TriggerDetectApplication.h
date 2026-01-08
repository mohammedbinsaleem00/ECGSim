/*
 * TriggerDetectApplication.h
 *
 *  Created on: 05-Jan-2026
 *      Author: Mohammed Bin Saleem
 */

#ifndef TRIGGERDETECTAPPLICATION_TRIGGERDETECTAPPLICATION_H_
#define TRIGGERDETECTAPPLICATION_TRIGGERDETECTAPPLICATION_H_

#include <stdint.h>
#include <stdbool.h>


void triggerProcess();
void pauseTriggerDetect();
void resumeTriggerDetect();
bool triggerDetectApplicationInit();

#endif /* TRIGGERDETECTAPPLICATION_TRIGGERDETECTAPPLICATION_H_ */
