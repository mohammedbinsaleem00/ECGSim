/*
 * ecgWaveGenerator.h
 *
 *  Created on: 29-Dec-2025
 *      Author: Mohammed Bin Saleem
 */

#ifndef UTILITIES_ECGWAVEGENERATOR_ECGWAVEGENERATOR_H_
#define UTILITIES_ECGWAVEGENERATOR_ECGWAVEGENERATOR_H_

typedef struct {
    float fs;      /* Sampling rate (Hz) */
    float hr;      /* Heart rate (bpm) */
    float noise;   /* Uniform noise amplitude (mV), 0 to disable */
} ecg_config_t;

int ecg_generate_beats(
    const ecg_config_t *cfg,
    int beats,
    float *ecg_out,
    int max_samples,
    int *r_peaks,
    int max_peaks
);

void isolate_beat(int total_samples, float* ecg_waveform, float* out_beat,
                  int* out_size);

int beat_peak_detect(int total_samples, float* ecg_waveform);

#endif /* UTILITIES_ECGWAVEGENERATOR_ECGWAVEGENERATOR_H_ */
