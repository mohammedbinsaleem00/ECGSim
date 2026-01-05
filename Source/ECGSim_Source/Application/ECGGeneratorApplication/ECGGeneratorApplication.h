/*
 * ECGGeneratorApplication.h
 *
 *  Created on: 29-Dec-2025
 *      Author: Mohammed Bin Saleem
 */

#ifndef ECGGENERATORAPPLICATION_ECGGENERATORAPPLICATION_H_
#define ECGGENERATORAPPLICATION_ECGGENERATORAPPLICATION_H_

#include <stdint.h>
#include <stdbool.h>


typedef enum{
	ECG_DOWNLOAD_STATE_IDLE,
	ECG_DOWNLOAD_IN_PROCESS,
	ECG_DOWNLOAD_STATE_COUNT
}ecgDownloadState_t;

void generateEcgWaveformData();
void exportEcg();
bool downloadEcgData(uint16_t currentProgress, uint16_t currentData);
bool initiateEcgDownload(uint16_t totalDownloadSize);
#endif /* ECGGENERATORAPPLICATION_ECGGENERATORAPPLICATION_H_ */
