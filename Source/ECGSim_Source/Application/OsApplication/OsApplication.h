/*
 * OsApplication.h
 *
 *  Created on: 30-Dec-2025
 *      Author: Mohammed Bin Saleem
 */

#ifndef OSAPPLICATION_OSAPPLICATION_H_
#define OSAPPLICATION_OSAPPLICATION_H_

#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "main.h"

#define BASIC_TASK_TIME_PERIOD_MS				1000
#define GENERATE_ECG_TASK_TIME_PERIOD_MS		1

void OsAppCreateTasks(void);
void OsAppLowerLayerInit(void);
void OsAppUpperLayerInit(void);

#endif /* OSAPPLICATION_OSAPPLICATION_H_ */
