/*
 * constants for gpiox
 *
 * (c) Derya Y. iiot2k@gmail.com
 *
 * gpio_def.h
 *
 */

#pragma once

#include <stdint.h>

// pin
#define N_PIN 28
#define CHECKPIN(p) (p<N_PIN)

// gpio mode
enum {
    GPIO_MODE_INPUT_NOPULL = 0,
    GPIO_MODE_INPUT_PULLDOWN,
    GPIO_MODE_INPUT_PULLUP,
    GPIO_MODE_OUTPUT,
    GPIO_MODE_OUTPUT_SOURCE,
    GPIO_MODE_OUTPUT_SINK,
};

#define DUTY_MIN 0
#define DUTY_MAX 100

#define FREQ_MIN 1
#define FREQ_MAX 1000
