/*
 * CLIApplication.h
 *
 *  Created on: 30-Dec-2025
 *      Author: Mohammed Bin Saleem
 */

#ifndef CLIAPPLICATION_CLIAPPLICATION_H_
#define CLIAPPLICATION_CLIAPPLICATION_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>


#define PRINT_DEBUG_MSG

#ifdef PRINT_DEBUG_MSG
#define DEBUG_PRINT_LINE(x) CLI_PrintLine(x)
#define DEBUG_PRINT(x,y)	CLI_Print(x,y)
#define DEBUG_PRINTF(formatedString, ...)	{\
	char data[100];		\
	uint32_t size = sprintf(data,formatedString, ##__VA_ARGS__);	\
	CLI_Print(data,size);	}\

#else
#define DEBUG_PRINT_LINE(x)
#define DEBUG_PRINT(x,y)
#define DEBUG_PRINTF(formatedString, ...)
#endif


/*Type definitions*/
typedef enum CommandStatus{
    E_NEW_COMMAND,
    E_COMMAND_ERROR,
    E_COMMAND_EXECUTED,
    E_NO_COMMAND,
    E_COMMAND_MANY_ARGS,
    E_COMMAND_NOT_FOUND,
    E_COMMAND_GOOD_COMMAND,
    E_COMMAND_BAD_COMMAND,
    E_COMMAND_FEW_ARGS,
    E_COMMAND_ARG_OK
}CommandStatus_t;

typedef int (*CommandHandlerFn_t)(int argc, char *argv[]);


typedef struct{
	const char *commandName;
	CommandHandlerFn_t commandHandler;
}CommandLineEntry_t;

/**
 * @brief Function to initialize the CLI
 */
bool CLI_Init();

/**
 * @brief Function processes new recieved characters and execute commands constructed out of them
 */
void CLI_Process();

/**
 * @brief Function will print the given character array to CLI
 * @param string is the character array to be printed
 */
void CLI_PrintLine(const char * string);

/**
 * @brief Function to print information to CLI over UART
 */
void CLI_Print(char* data,uint32_t size);



#endif /* CLIAPPLICATION_CLIAPPLICATION_H_ */
