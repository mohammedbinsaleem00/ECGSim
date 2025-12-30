/*
 * OsApplication.c
 *
 *  Created on: 30-Dec-2025
 *      Author: Mohammed Bin Saleem
 */


#include "OsApplication.h"
#include "ECGGeneratorApplication.h"
#include "VoltageController.h"

osThreadId_t basicTaskHandle;

const osThreadAttr_t basicTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

osThreadId_t generateEcgTaskHandle;

const osThreadAttr_t generateEcgTask_attributes = {
  .name = "generateEcgTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};

void basicTask(void *argument)
{
	for(;;)
	{
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
		osDelay(BASIC_TASK_TIME_PERIOD_MS);
	}
}

void generateEcgTask(void* argument)
{
	for(;;)
	{
		generateEcg();
		osDelay(GENERATE_ECG_TASK_TIME_PERIOD_MS);
	}
}

void OsAppCreateTasks(void)
{
	basicTaskHandle = osThreadNew(basicTask, NULL, &basicTask_attributes);
	generateEcgTaskHandle = osThreadNew(basicTask, NULL, &basicTask_attributes);
}

void OsAppLowerLayerInit(void)
{
	VoltageControllerInit();
}

void OsAppUpperLayerInit(void)
{
	ecgGeneratorAppInit();
}
