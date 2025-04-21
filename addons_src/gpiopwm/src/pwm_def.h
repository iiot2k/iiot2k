/*
 * constants for gpiopwm
 *
 * (c) Derya Y. iiot2k@gmail.com
 *
 * pwm_def.h
 *
 */

#pragma once

#include <stdint.h>

// pin
#define N_PIN 28
#define CHECKPIN(p) (p<N_PIN)

#define DUTY_MIN 0   // min. duty cycle (%)
#define DUTY_MAX 100 // max. duty cycle (%)

#define FREQ_MIN 1     // min. pwm frequency (Hz)
#define FREQ_MAX 45000 // max. pwm frequency (Hz)
