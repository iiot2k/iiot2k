/*
 * constants for gpio_mon
 *
 * (c) Derya Y. iiot2k@gmail.com
 *
 * gpiomon_def.h
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
};

// gpio edge mode
enum {
    GPIO_EDGE_RISING = 0,
    GPIO_EDGE_FALLING,
    GPIO_EDGE_BOTH,
};
