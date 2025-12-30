/*
 * ecgWaveGenerator.c
 *
 *  Created on: 29-Dec-2025
 *      Author: Mohammed Bin Saleem
 */

#include <math.h>
#include <stdint.h>
#include <string.h>
#include "ecgWaveGenerator.h"

/* ========================= CONFIG ========================= */

#define PI 3.14159265358979323846f
#define Z_RECOVERY_GAIN  6.0f

/* ========================= TYPES ========================== */

/* typedef struct already defined in header */

/* ========================= MORPHOLOGY ===================== */
/* P, Q, R, S, T parameters (from ECGSYN) */

static const float ti[5] = {
    -60.0f * PI / 180.0f,
    -15.0f * PI / 180.0f,
      0.0f,
     15.0f * PI / 180.0f,
     90.0f * PI / 180.0f
};

static const float ai[5] = {
     1.2f,   /* P */
    -5.0f,   /* Q */
    30.0f,   /* R */
    -7.5f,   /* S */
     0.75f   /* T */
};

static const float bi[5] = {
    0.25f,
    0.10f,
    0.10f,
    0.10f,
    0.40f
};

/* ========================= RNG (optional noise) ============ */

static uint32_t rng_state = 1;

static float rand_uniform(void)
{
    rng_state = 1664525UL * rng_state + 1013904223UL;
    return ((rng_state >> 8) & 0xFFFFFF) / 16777216.0f;
}

/* ========================= ECG DERIVATIVE ================= */

static void ecg_derivative(
    const float x[3],
    float dxdt[3],
    float w
)
{
    float r = sqrtf(x[0]*x[0] + x[1]*x[1]);
    float a = 1.0f - r;

    /* x-y plane (limit cycle) */
    dxdt[0] = a*x[0] - w*x[1];
    dxdt[1] = a*x[1] + w*x[0];

    /* z (ECG morphology) */
    float theta = atan2f(x[1], x[0]);
    float z = 0.0f;

    for (int i = 0; i < 5; i++) {
        float dt = theta - ti[i];

        /* wrap to [-pi, pi] */
        if (dt > PI)  dt -= 2.0f*PI;
        if (dt < -PI) dt += 2.0f*PI;

        z += -ai[i] * dt * expf(-0.5f * dt * dt / (bi[i]*bi[i]));
    }

    dxdt[2] = Z_RECOVERY_GAIN * (z - x[2]);
    // dxdt[2] = z - x[2];
}

/* ========================= RK4 INTEGRATOR ================= */

static void rk4_step(float x[3], float h, float w)
{
    float k1[3], k2[3], k3[3], k4[3], xt[3];

    ecg_derivative(x, k1, w);

    for (int i = 0; i < 3; i++)
        xt[i] = x[i] + 0.5f*h*k1[i];
    ecg_derivative(xt, k2, w);

    for (int i = 0; i < 3; i++)
        xt[i] = x[i] + 0.5f*h*k2[i];
    ecg_derivative(xt, k3, w);

    for (int i = 0; i < 3; i++)
        xt[i] = x[i] + h*k3[i];
    ecg_derivative(xt, k4, w);

    for (int i = 0; i < 3; i++) {
        x[i] += (h / 6.0f) *
                (k1[i] + 2.0f*k2[i] + 2.0f*k3[i] + k4[i]);
    }
}

/* ========================= PUBLIC API ===================== */

/*
 * Generate ECG waveform for a fixed number of beats.
 *
 * ecg_out     : output ECG buffer (mV)
 * max_samples : capacity of ecg_out[]
 * r_peaks     : output R-peak indices
 * max_peaks   : capacity of r_peaks[]
 *
 * returns number of generated samples
 */
int ecg_generate_beats(
    const ecg_config_t *cfg,
    int beats,
    float *ecg_out,
    int max_samples,
    int *r_peaks,
    int max_peaks
)
{
    float rr = 60.0f / cfg->hr;
    float w = 2.0f * PI / rr;
    float h = 1.0f / cfg->fs;

    int samples_per_beat = (int)(rr * cfg->fs);
    int total_samples = beats * samples_per_beat;
    if (total_samples > max_samples)
        total_samples = max_samples;

    /* Initial state */
    float x[3] = { 1.0f, 0.0f, 0.04f };

    float prev_theta = atan2f(x[1], x[0]);
    int peak_count = 0;

    for (int i = 0; i < total_samples; i++) {
        rk4_step(x, h, w);

        float z = x[2];

        /* optional noise */
        if (cfg->noise > 0.0f) {
            z += cfg->noise * (2.0f*rand_uniform() - 1.0f);
        }

        ecg_out[i] = z;

        /* R-peak detection (theta crossing 0) */
        float theta = atan2f(x[1], x[0]);
        if (prev_theta < 0.0f && theta >= 0.0f) {
            if (peak_count < max_peaks)
                r_peaks[peak_count++] = i;
        }
        prev_theta = theta;
    }


    return total_samples;
}

void isolate_beat(int total_samples, float* ecg_waveform, float* out_beat,
                  int* out_size)
{
    *out_size = total_samples / 2;
    int start = total_samples / 4;
    int end = start + *out_size;

    /* Copy waveform data */
    memcpy(out_beat, ecg_waveform + start, (*out_size) * sizeof(float));

}

int beat_peak_detect(int total_samples, float* ecg_waveform)
{
    float max_value = -9999999;
    int peak_index = -1;

    for (int i = 0; i < total_samples; i++) {
        if (ecg_waveform[i] > max_value) {
            max_value = ecg_waveform[i];
            peak_index = i;
        }
    }

    return peak_index;
}
