/**
 * @file Commands.c
 * @author Developer, Tismo
 * @date 26-May-2020
 * @brief This file provides commands for CLI over serial
 *
 * @attention COPYRIGHT(c) 2020 MIPM GMBH
 */



#include "CLIApplication.h"
#include "ECGGeneratorApplication.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>



#define PRINT_CLI_MSG

#ifdef PRINT_CLI_MSG
#define CLI_PRINT_LINE(x) CLI_PrintLine(x)
#define CLI_PRINT(x,y)	CLI_Print(x,y)
#define CLI_PRINTF(formatedString, ...)	{\
		char data[100];		\
		uint32_t size = sprintf(data,formatedString, ##__VA_ARGS__);	\
		CLI_Print(data,size);	}\

#else
#define CLI_PRINT_LINE(x)
#define CLI_PRINT(x,y)
#define CLI_PRINTF(formatedString, ...)
#endif

#define MAX_BAUD_RATE 1000000

#define COMMAND_PRINT_FIRMWARE_INFO 	"GetFirmwareInfo"
#define COMMAND_INITIATE_ECG_DOWNLOAD 	"InitiateEcgDownload"
#define COMMAND_DOWNLOAD_ECG_DATA 		"DownloadEcgData"



//Encryption Test Commands
#define RParameterCount  4

char ackText[5] = "\nok";


static int printFirmwareInfo(int argc, char* argv[]);
static int initiateEcgDownloadFn(int argc, char * argv[]);
static int ecgDownloadFn(int argc, char * argv[]);

const CommandLineEntry_t g_commandTable[]={
		{COMMAND_PRINT_FIRMWARE_INFO, printFirmwareInfo},
		{COMMAND_INITIATE_ECG_DOWNLOAD, initiateEcgDownloadFn},
		{COMMAND_DOWNLOAD_ECG_DATA, ecgDownloadFn},
		{0,0} // End of List. Always required
};



int printFirmwareInfo(int argc, char * argv[])
{
	char firmwareInfo[150] = "\nECGSIM Protoype V0.1";



	CLI_Print(firmwareInfo, strlen(firmwareInfo));

	return E_COMMAND_GOOD_COMMAND;
}


int initiateEcgDownloadFn(int argc, char * argv[])
{
	if(argc < 2)
	{
		return E_COMMAND_FEW_ARGS;
	}

	uint32_t downloadSize = 0;
	sscanf(argv[1],"%lu",&downloadSize);


	if(initiateEcgDownload((uint16_t)downloadSize))
	{
		CLI_Print(ackText, strlen(ackText));
		return E_COMMAND_GOOD_COMMAND;
	}
	else
		return E_COMMAND_BAD_COMMAND;
}

int ecgDownloadFn(int argc, char * argv[])
{
	if(argc < 3)
	{
		return E_COMMAND_FEW_ARGS;
	}

	uint32_t ecgData = 0;
	uint32_t dataIndex = 0;
	sscanf(argv[1],"%lu",&dataIndex);
	sscanf(argv[2],"%lu",&ecgData);

	if(downloadEcgData((uint16_t)dataIndex,(uint16_t)ecgData))
	{
		CLI_Print(ackText, strlen(ackText));
		return E_COMMAND_GOOD_COMMAND;
	}
	else
		return E_COMMAND_BAD_COMMAND;
}
