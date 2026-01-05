/*
 * CLIApplication.c
 *
 *  Created on: 30-Dec-2025
 *      Author: Mohammed Bin Saleem
 */


#include "CLIApplication.h"


#include <stdio.h>
#include <string.h>

#include "customUART.h"


/**
 * @addtogroup CLI
 * @{
 */

/**
 * @defgroup SERIAL_CLI CLI over Serial
 * @{
 */


/**
 * @defgroup SERIAL_CLI_PVT_DEFS CLI over Serial Private definitions
 * @{
 */
/*Macros*/
#define COMMAND_BUFFER_SIZE 100
//Maximum value of MAX_ARGS is 255
#define COMMAND_MAX_ARGS 20

#define CLI_INFO_STRING "\nECGSIM Protoype - Command Line Interface\n"
#define COMMAND_NOT_FOUND_ERROR_STRING "Command not found\n"
#define COMMAND_FEW_ARGS_ERROR_STRING "Insufficient number of arguments\n"
#define COMMAND_BAD_ERROR_STRING "Command Bad\n"
#define COMMAND_EXECUTED_STRING "Command Executed\n"
#define COMMAND_TOO_LONG "Command length too long"
#define COMMAND_END_CHARACTER '\r'
#define COMMAND_ARG_SEPARATOR ' '

/**@}*/ // SERIAL_CLI_PVT_DEFS


/**
 * @defgroup SERIAL_CLI_PVT_TYPES_N_VARS  CLI over Serial Private Types and Variables
 * @{
 */
/*Global variables*/


struct commandBuffer{
	char buffer[COMMAND_BUFFER_SIZE];
	uint32_t currentIndex;
}g_commandBuffer;

struct argument{
	uint8_t count;
	char *list[COMMAND_MAX_ARGS];
}g_arguments;

bool g_argumentFound = true;
extern const CommandLineEntry_t g_commandTable[];

/**@}*/ // SERIAL_CLI_PVT_TYPES_N_VARS

/**
 * @defgroup SERIAL_CLI_PVT_FUNCS  CLI over Serial Private functions
 * @{
 */
#ifndef UNIT_TEST

/*Static functions*/
static void resetCommandBuffer();
static void addToCommandBuffer(char a_data);
static CommandStatus_t constructCommand();
static CommandStatus_t executeCommand();
static void printStatus(CommandStatus_t status);
static void printCLIInfo();
static void resetArgumentCount();

//CLI Comm functions
static bool getCharacter(char *receivedCharacter);
static bool sendData(char *data,uint32_t size);

#endif

/**
 * @brief Function to construct command from the characters
 * @return Command Status
 */
CommandStatus_t constructCommand()
{
	char receivedCharacter;

	if(getCharacter(&receivedCharacter))
	{
		if(receivedCharacter == COMMAND_END_CHARACTER)
		{
			//Reset argument found to true for next command
			addToCommandBuffer('\0');
			g_argumentFound = true;

			return E_NEW_COMMAND;
		}

		if(receivedCharacter == COMMAND_ARG_SEPARATOR)
		{
			addToCommandBuffer('\0');
			g_argumentFound = true;
		}
		else
		{
			addToCommandBuffer(receivedCharacter);

			if(g_argumentFound)
			{
				if(g_arguments.count < COMMAND_MAX_ARGS)
				{
					g_arguments.list[g_arguments.count] =
							&g_commandBuffer.buffer[g_commandBuffer.currentIndex - 1];

					g_arguments.count++;
					g_argumentFound = false;
				}
				else
				{
					return E_COMMAND_MANY_ARGS;
				}
			}
		}
	}
	return E_NO_COMMAND;
}

/**
 * @brief Function is used to process the command received
 * @return Command Status
 */

CommandStatus_t executeCommand()
{
	const CommandLineEntry_t *commandLineEntry;
#ifndef UNIT_TEST
	commandLineEntry = &g_commandTable[0];
	while(commandLineEntry->commandName)
	{
		//Call command handler if the command name matches
		if(!strcmp(commandLineEntry->commandName, g_arguments.list[0]))
		{
			return commandLineEntry->commandHandler(g_arguments.count,
					g_arguments.list);
		}

		commandLineEntry++;
	}

	return E_COMMAND_NOT_FOUND;
#endif
	return E_COMMAND_ARG_OK;
}

/**
 * @brief Function is used to reset the command buffer
 * @return NONE
 */
void resetCommandBuffer()
{
	memset(g_commandBuffer.buffer, 0, COMMAND_BUFFER_SIZE);
	g_commandBuffer.currentIndex = 0;
}

/**
 * @brief Function resets the argument count
 * @return NONE
 */
void resetArgumentCount()
{
	g_arguments.count = 0;
}

/**
 * @brief Function is used to add characters to the global
 * command buffer
 * @param a_data : Character to be added to the buffer
 * @return NONE
 */
void addToCommandBuffer(char a_data)
{
	if(g_commandBuffer.currentIndex >= COMMAND_BUFFER_SIZE)
	{
		CLI_PrintLine(COMMAND_TOO_LONG);
		resetCommandBuffer();
		return ;
	}
	g_commandBuffer.buffer
	[g_commandBuffer.currentIndex++] = a_data;
}

/**
 * @brief Function to print CLI info
 */
void printCLIInfo()
{
	CLI_PrintLine(CLI_INFO_STRING);

	char firmwareInfo[250] = "ECGSIM Protoype V0.1";



	CLI_Print(firmwareInfo, strlen(firmwareInfo));
}

/**
 * @brief Function to print status of command handler
 * @param status Status from command handler
 */
void printStatus(CommandStatus_t status)
{
	switch(status)
	{
	case E_COMMAND_NOT_FOUND: CLI_PrintLine(COMMAND_NOT_FOUND_ERROR_STRING);
	break;
	case E_COMMAND_FEW_ARGS: CLI_PrintLine(COMMAND_FEW_ARGS_ERROR_STRING);
	break;
	case E_COMMAND_BAD_COMMAND: CLI_PrintLine(COMMAND_BAD_ERROR_STRING);
	break;

	//Add more status print messages if required
	default:break;
	}
}

/**
 * @brief Function to fetch data from serial line
 * @param receivedCharacter Pointer to get the received data from Serial buffers
 * @return Status of function call
 * @retval 0 Failure
 * @retval 1 Success
 */
bool getCharacter(char *receivedCharacter)
{
	return UART_ReadByteNonBlocking(DEBUG_UART, (uint8_t *)receivedCharacter);
}

/**
 * @brief Function to send data over serial
 * @param data Pointer to the data to be send of type uint8_t
 * @param size Size of message
 * @return Status of function call
 * @retval 0 Failure
 * @retval 1 Success
 */
bool sendData(char *data,uint32_t size)
{
	return UART_Write(DEBUG_UART, (uint8_t*)data, size);
}

/**@}*/ // SERIAL_CLI_PVT_FUNCS


/**
 * @defgroup SERIAL_CLI_PUB_FUNC CLI over Serial public functions
 * @{
 */


/**
 * @brief Function is used to initialize the CLI over UART
 * @return Status of function call
 * @retval 0 Failure
 * @retval 1 Success
 */
bool CLI_Init()
{
	resetCommandBuffer();

	resetArgumentCount();

	printCLIInfo();

	return true;
}

/**
 * @brief CLI task function
 * @note Check for new commands and process if any new commands are received
 */
void CLI_Process()
{
	CommandStatus_t status;

	if(constructCommand() != E_NEW_COMMAND)
	{
		return;
	}

	status = executeCommand();

	printStatus(status);

	resetCommandBuffer();

	resetArgumentCount();
}

/**
 * @brief Function is used to print null terminated string
 * @param a_string Pointer to the null terminated string
 * @return none
 */
void CLI_PrintLine(const char *a_string)
{
	CLI_Print((char *)a_string, strlen(a_string));
}

/**
 * @brief Function to print information to CLI
 * @param a_data Pointer to the character array to be printed
 * @param a_count Number of characters to be printed
 * @return none
 */
void CLI_Print(char* data,uint32_t size)
{
	sendData(data,size);
}
