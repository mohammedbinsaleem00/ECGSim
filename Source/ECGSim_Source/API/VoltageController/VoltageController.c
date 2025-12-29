/*
 * VoltageController.c
 *
 *  Created on: 26-Dec-2025
 *      Author: mohammed
 */

#include "VoltageController.h"
#include "i2c.h"
#include "MCP4725.h"


MCP4725 VoltageControllerDevice;

void VoltageControllerInit()
{
	 MX_I2C1_Init();
	 VoltageControllerDevice = MCP4725_init(&hi2c1, MCP4725A0_ADDR_A00, REF_VOLTAGE);
}

bool VoltageControllerProbe()
{
	return MCP4725_isConnected(&VoltageControllerDevice);
}

void VoltageControllerSetVoltage(uint16_t value)
{
	MCP4725_setValue(&VoltageControllerDevice, value, MCP4725_FAST_MODE, MCP4725_POWER_DOWN_OFF);
}
