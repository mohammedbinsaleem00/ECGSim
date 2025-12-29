/*
 * VoltageController.h
 *
 *  Created on: 26-Dec-2025
 *      Author: mohammed
 */

#ifndef API_VOLTAGECONTROLLER_VOLTAGECONTROLLER_H_
#define API_VOLTAGECONTROLLER_VOLTAGECONTROLLER_H_

#include <stdint.h>
#include <stdbool.h>

#define REF_VOLTAGE			3.30

void VoltageControllerInit(void);
bool VoltageControllerProbe(void);
void VoltageControllerSetVoltage(uint16_t value);

#endif /* API_VOLTAGECONTROLLER_VOLTAGECONTROLLER_H_ */
