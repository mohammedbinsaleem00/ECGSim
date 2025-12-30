/*
 * CommonConfigurations.h
 *
 *  Created on: 29-Dec-2025
 *      Author: Mohammed Bin Saleem
 */

#ifndef UTILITIES_COMMONCONFIGURATIONS_H_
#define UTILITIES_COMMONCONFIGURATIONS_H_

#define FLT_MAX 3.402823466e+38F  /* max finite value of 32-bit IEEE-754 float */

#define MAX_SAMPLES 2000
#define MAX_PEAKS 5


#define DAC_VREF              3.3f
#define DAC_BITS              12
#define DAC_MAX_CODE          ((1 << DAC_BITS) - 1)
#define DAC_MID_CODE          (DAC_MAX_CODE / 2)   // 2047 or 2048
#define DIVIDER_RATIO         (10.0f / (10000.0f + 10.0f))  // ≈ 0.001

#define ECG_PP_MV_TARGET      3.0f    // 3 mV p-p at RA–LA
#define ECG_DAC_PP_VOLTS  (ECG_PP_MV_TARGET * 1e-3f / DIVIDER_RATIO)
#define ECG_DAC_PP_CODES  ((ECG_DAC_PP_VOLTS / DAC_VREF) * DAC_MAX_CODE)
#define ECG_DAC_HALF_SWING  (ECG_DAC_PP_CODES / 2.0f)   // ≈ 1863


#endif /* UTILITIES_COMMONCONFIGURATIONS_H_ */
