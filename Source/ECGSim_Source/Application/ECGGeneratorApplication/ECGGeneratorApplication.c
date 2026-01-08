/*
 * ECGGeneratorApplication.c
 *
 *  Created on: 29-Dec-2025
 *      Author: Mohammed Bin Saleem
 */

#include "ECGGeneratorApplication.h"
#include "ecgWaveGenerator.h"
#include "CommonConfigurations.h"
#include "VoltageController.h"
#include "TriggerDetectApplication.h"
#include "Stopwatch.h"


ecg_config_t g_ecgConfig = {
        .fs = 1000.0f,   /* sampling rate */
        .hr = 300.0f,    /* bpm */
        .noise = 0.00f  /* mV noise */
    };



uint16_t g_rawControllerData[MAX_SAMPLES];
int g_Rpeaks[MAX_PEAKS];
int g_peakIndex = MAX_SAMPLES + 1;
int g_waveformSize = 0;
int g_waveformIndex = 0;
bool g_ecgUpdateRequired = true;
uint16_t g_ecgDownloadTotalSize = 0;
uint16_t g_ecgDownloadProgress = 0;
ecgDownloadState_t g_ecgDownloadState = ECG_DOWNLOAD_STATE_IDLE;

//float g_rawEcgData[MAX_SAMPLES];

static void scaleTo3mVpp(
    const float *ecg_in,     // real ECG waveform
    uint16_t length,
    uint16_t *dac_out        // 12-bit DAC output
)
{
    float min = FLT_MAX;
    float max = -FLT_MAX;

    /* Find min and max */
    for (uint16_t i = 0; i < length; i++) {
        if (ecg_in[i] < min) min = ecg_in[i];
        if (ecg_in[i] > max) max = ecg_in[i];
    }

    float range = max - min;

    /* Safety check */
    if (range < 1e-9f) {
        for (uint16_t i = 0; i < length; i++)
            dac_out[i] = DAC_MID_CODE;
        return;
    }

    /* Normalize and scale */
    for (uint16_t i = 0; i < length; i++) {
        float norm = (ecg_in[i] - min) / range;  // 0 → 1
        norm = (norm * 2.0f) - 1.0f;              // -1 → +1

        float dac_f =
            (float)DAC_MID_CODE +
            (norm * ECG_DAC_HALF_SWING);

        /* Clamp */
        if (dac_f < 0.0f) dac_f = 0.0f;
        if (dac_f > DAC_MAX_CODE) dac_f = DAC_MAX_CODE;

        dac_out[i] = (uint16_t)(dac_f + 0.5f);
    }
}

void generateEcgWaveformData()
{
//	float rawEcgData[MAX_SAMPLES];
//	float isolatedBeatData[MAX_SAMPLES];

//	int waveformSize = ecg_generate_beats(&g_ecgConfig, 2, rawEcgData, MAX_SAMPLES, g_Rpeaks, MAX_PEAKS);
//	isolate_beat(waveformSize, rawEcgData, isolatedBeatData, &waveformSize);
//	int waveformSize = g_ecgDownloadTotalSize;
//	g_peakIndex = beat_peak_detect(waveformSize, g_rawEcgData);
//	scaleTo3mVpp(isolatedBeatData, waveformSize, (uint16_t*)g_rawControllerData);
//	g_waveformSize = waveformSize;
//
//	g_ecgUpdateRequired = false;

}

int beatPeakDetect(int total_samples, uint16_t* ecg_waveform)
{
    uint16_t max_value = 0;
    int peak_index = -1;

    for (int i = 0; i < total_samples; i++) {
        if (ecg_waveform[i] > max_value) {
            max_value = ecg_waveform[i];
            peak_index = i;
        }
    }

    return peak_index;
}

void exportEcg()
{
	if(g_ecgDownloadState != ECG_DOWNLOAD_STATE_IDLE)
	{
		return;
	}

	
	if (g_waveformIndex < g_waveformSize)
	{
		if(g_waveformIndex == g_peakIndex)
		{
			startStopwatch(&triggerSw);
		}
		VoltageControllerSetRawVoltage(g_rawControllerData[g_waveformIndex++]);
	}
	else
	{
		g_waveformIndex = 0;
	}
}

bool isEcgUpdateRequired()
{
	return g_ecgUpdateRequired;
}

void setEcgUpdateRequired(bool status)
{
	g_ecgUpdateRequired = status;
}



bool initiateEcgDownload(uint16_t totalDownloadSize)
{
	if(g_ecgDownloadState == ECG_DOWNLOAD_STATE_IDLE)
	{
		g_ecgDownloadTotalSize = totalDownloadSize;
		g_ecgDownloadProgress = 0;
		g_ecgDownloadState = ECG_DOWNLOAD_IN_PROCESS;
		pauseTriggerDetect();
		return true;
	}
	return false;
}
bool downloadEcgData(uint16_t currentProgress, uint16_t currentData)
{
	bool status = false;
	switch(g_ecgDownloadState)
	{
	case ECG_DOWNLOAD_STATE_IDLE:
		status = false;
		break;
	case ECG_DOWNLOAD_IN_PROCESS:
		if (currentProgress == g_ecgDownloadProgress )
		{
			g_rawControllerData[g_ecgDownloadProgress++] = currentData;

			if(g_ecgDownloadProgress == g_ecgDownloadTotalSize)
			{
				g_ecgDownloadState = ECG_DOWNLOAD_STATE_IDLE;
				g_waveformSize = g_ecgDownloadTotalSize;
				g_peakIndex = beatPeakDetect(g_waveformSize, g_rawControllerData);
				resumeTriggerDetect();
			}
			status = true;
		}
		break;
	default:
		status = false;
		break;

	}
	return status;
}
