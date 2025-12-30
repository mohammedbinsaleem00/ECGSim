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




ecg_config_t g_ecgConfig = {
        .fs = 1000.0f,   /* sampling rate */
        .hr = 60.0f,    /* bpm */
        .noise = 0.00f  /* mV noise */
    };



uint16_t g_rawControllerData[MAX_SAMPLES];
int g_Rpeaks[MAX_PEAKS];
int g_peakIndex = MAX_SAMPLES + 1;
int g_waveformSize = 0;
int g_waveformIndex = 0;

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

static void generateEcgWaveformData()
{
	float rawEcgData[MAX_SAMPLES];
	float isolatedBeatData[MAX_SAMPLES];

	int waveformSize = ecg_generate_beats(&g_ecgConfig, 2, rawEcgData, MAX_SAMPLES, g_Rpeaks, MAX_PEAKS);
	isolate_beat(waveformSize, rawEcgData, isolatedBeatData, &waveformSize);
	g_peakIndex = beat_peak_detect(waveformSize,isolatedBeatData);
	scaleTo3mVpp(isolatedBeatData, waveformSize, (uint16_t*)g_rawControllerData);
	g_waveformSize = waveformSize;

}

void ecgGeneratorAppInit()
{
	generateEcgWaveformData();
}

void generateEcg()
{
	if (g_waveformIndex < g_waveformSize)
	{
		VoltageControllerSetRawVoltage(g_rawControllerData[g_waveformIndex++]);
	}
	else
	{
		g_waveformIndex = 0;
	}
}
