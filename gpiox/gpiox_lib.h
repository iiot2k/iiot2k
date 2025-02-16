/*
 * 
 * (c) Derya Y. iiot2k@gmail.com
 *
 * Raspberry Pi gpio c++ library
 *
 * the library uses the gpio character devices interface (V2) from the Linux operating system
 * 
 * link library with your application
 * libgpiox_arm64.a on 64bit OS
 *
 * gpiox_lib.h
 *
 */

#pragma once

#include <vector>
#include <string>
using namespace std;

#include <stdint.h>

namespace gpiox {

/**
 * @brief gets error text after call functions
 * @returns error text
 */
const char* error_text();

/************** GPIO initialization functions **************/

/**
 * @brief gpio modes
 * @note floating input/output is used when the pin is connected to another pin
 * input voltages more than +3.3V can destroy the input
 * Hi-Z refers to an output signal state in which the signal is not being driven
 * please do not init gpio pins with special functions (i2c, spi, uart..)
 * gpio output current is limited
 * use ULN2803A (low switch) or TBD62783 (high switch) for drive output
 */
enum {
    GPIO_MODE_INPUT_NOPULL = 0, // floating input (used in init_gpio or change_gpio)
    GPIO_MODE_INPUT_PULLDOWN,   // pulldown resistor input (used in init_gpio or change_gpio)
    GPIO_MODE_INPUT_PULLUP,     // pullup resistor input (used in init_gpio or change_gpio)
    GPIO_MODE_OUTPUT,           // output (used in init_gpio or change_gpio)
    GPIO_MODE_OUTPUT_SOURCE,    // output source (Hi-Z on 0) (used in init_gpio or change_gpio)
    GPIO_MODE_OUTPUT_SINK,      // output sink (Hi-Z on 0) (used in init_gpio or change_gpio)
};

/**
 * @brief init gpio input/output with mode
 * @param pin gpio pin (0..27)
 * @param mode GPIO_MODE_INPUT_.. or GPIO_MODE_OUTPUT..
 * @param setval debounce time in us for inputs, 0/1 or true/false for output
 * @returns true on ok, false on error (error_text() returns reason)
 * @note setval 0 for inputs disables debounce
 */
bool init_gpio(uint32_t pin, uint32_t mode, uint32_t setval);

/**
 * @brief deinit gpio
 * @param pin gpio pin (0..27)
 * @returns true on ok, false on error (error_text() returns reason)
 * @note stops also blink)
 */
bool deinit_gpio(uint32_t pin);

/************** GPIO Input/Output functions **************/

/**
 * @brief get gpio state
 * @param pin gpio pin (0..27)
 * @returns true on active, false on deactive
 */
bool get_gpio(uint32_t pin);

/**
 * @brief get gpio state as number (0/1)
 * @param pin gpio pin (0..27)
 * @returns 1 on active, 0 on deactive
 */
uint32_t get_gpio_num(uint32_t pin);

/**
 * @brief set gpio output
 * @param pin gpio pin (0..27)
 * @param val 0/1 or true/false
 * @returns true on ok, false on error (error_text() returns reason)
 */
bool set_gpio(uint32_t pin, uint32_t val);

/**
 * @brief toggle (change state) of gpio output
 * @param pin gpio pin (0..27)
 * @returns true on ok, false on error (error_text() returns reason)
 */
bool toggle_gpio(uint32_t pin);

/**
 * @brief blink gpio output on period
 * @param pin gpio pin (0..27)
 * @param period blink period in ms (1..), 0 stops blink
 * @returns true on ok, false on error (error_text() returns reason)
 */
bool blink_gpio(uint32_t pin, uint32_t period);

/**
 * @brief stop gpio blink and set output to 0
 * @param pin gpio pin (0..27)
 * @returns true on ok, false on error (error_text() returns reason)
 * @note call of read/write/toggle functions stops also blink
 */
bool stop_blink_gpio(uint32_t pin);

} // namespace
