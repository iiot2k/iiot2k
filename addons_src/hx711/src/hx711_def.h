/*
 * constants for hx711
 *
 * (c) Derya Y. iiot2k@gmail.com
 *
 * hx711_def.h
 *
 */

#pragma once

#include <stdint.h>

// gain and channel
enum {
    GAIN_A128 = 1, // channel A gain 128
    GAIN_B32 = 2,  // channel B gain 32
    GAIN_A64 = 3,  // channel A gain 64
};
