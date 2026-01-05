/*
 * OsApplication.c
 *
 *  Created on: 30-Dec-2025
 *      Author: Mohammed Bin Saleem
 */


#include "OsApplication.h"
#include "ECGGeneratorApplication.h"
#include "VoltageController.h"
#include "customUART.h"
#include "CLIApplication.h"

osThreadId_t basicTaskHandle;
const osThreadAttr_t basicTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};





osThreadId_t ecgWorkerTaskHandle;
const osThreadAttr_t ecgWorkerTask_attributes = {
  .name = "ecgWorkerTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t cliTaskHandle;
const osThreadAttr_t cliTask_attributes = {
  .name = "cliTask",
  .stack_size = 1024,//128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};


void basicTask(void *argument)
{
    uint32_t lastWakeTime = osKernelGetTickCount();

    for (;;)
    {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        osDelayUntil(lastWakeTime + BASIC_TASK_TIME_PERIOD_MS);
        lastWakeTime += BASIC_TASK_TIME_PERIOD_MS;
    }
}


void ecgWorkerTask(void *argument)
{

    uint32_t lastWakeTime = osKernelGetTickCount();
    for (;;)
    {

        exportEcg();

        osDelayUntil(lastWakeTime + GENERATE_ECG_TASK_TIME_PERIOD_MS);
        lastWakeTime += GENERATE_ECG_TASK_TIME_PERIOD_MS;
    }
}

void cliTask(void *argument)
{
	for(;;)
	{
		CLI_Process();
	}
}
void OsAppCreateTasks(void)
{
	basicTaskHandle = osThreadNew(basicTask, NULL, &basicTask_attributes);
	cliTaskHandle = osThreadNew(cliTask, NULL, &cliTask_attributes);
	ecgWorkerTaskHandle = osThreadNew(ecgWorkerTask, NULL, &ecgWorkerTask_attributes);

	configASSERT(basicTaskHandle != NULL);
	configASSERT(cliTaskHandle != NULL);
	configASSERT(ecgWorkerTaskHandle != NULL);
}

void OsAppLowerLayerInit(void)
{
	UART_Init(DEBUG_UART);
	VoltageControllerInit();
}

void OsAppUpperLayerInit(void)
{
//	ecgGeneratorAppInit();
}
