/*
 * n-api C++ node.js interface
 *
 * (c) Derya Y. iiot2k@gmail.com
 *
 * node.cpp
 *
 */

#include <napi.h>
using namespace Napi;

#include "stdio.h"

#include "c_gpio.h"
#include "gpio_def.h"
#include "exception.h"
#include "error_code.h"

#define MODULE_VERSION "3.0.5"

#define OUTTEXT "@iiot2k/gpiox version: " MODULE_VERSION

// print banner
__attribute__((constructor)) static void libstart()
{
    puts(OUTTEXT);
}

// storage for gpio items
c_gpioitem gpio_item[N_PIN];

/**
 * returns addon version.
 */
Value version(const CallbackInfo &info)
{
    return String::New(info.Env(), MODULE_VERSION);
}

/**
 * returns error text on failure.
 */
Value error_text(const CallbackInfo &info)
{
    return String::New(info.Env(), get_error_text());
}

/**
* deinits gpio.
*
* @param pin - gpio pin number 0..27
* @returns true on ok, false on error
*
* @note GPIO pin is deinitialized when the program ends
* @note stops also blink, pulse, pwm
*/
Value deinit_gpio(const CallbackInfo &info)
{
    clear_error();
    CHECK_PARAM(1)
    uint32_t pin = info[0].ToNumber().Uint32Value();

    if (pin >= N_PIN)
    {
        set_error(ERR_PIN);
        return Boolean::New(info.Env(), false);
    }

    gpio_item[pin].deinit();

    return Boolean::New(info.Env(), true);
}

/**
 * inits gpio.
 *
 * @param pin - gpio pin number 0..27
 * @param mode - constant GPIO_MODE_..
 * @param setval - debounce time in us for input, state to set 0/1 or true/false for output
 * @returns true on ok, false on error
 *
 * @note setval 0 for inputs disables debounce
 */
Value init_gpio(const CallbackInfo &info)
{
    clear_error();
    CHECK_PARAM(3)
    uint32_t pin    = info[0].ToNumber().Uint32Value();
    uint32_t mode   = info[1].ToNumber().Uint32Value();
    uint32_t setval = info[2].ToNumber().Uint32Value();

    if (pin >= N_PIN)
    {
        set_error(ERR_PIN);
        return Boolean::New(info.Env(), false);
    }

    return Boolean::New(info.Env(), gpio_item[pin].init(pin, mode, setval));
}

/**
 * gets gpio state as boolean.
 *
 * @param pin - gpio pin number 0..27
 * @returns true/false, undefined on error
 */
Value get_gpio(const CallbackInfo &info)
{
    clear_error();
    CHECK_PARAM(1)
    uint32_t pin = info[0].ToNumber().Uint32Value();
    uint32_t val;

    if (pin >= N_PIN)
    {
        set_error(ERR_PIN);
        return Boolean::New(info.Env(), false);
    }

    if (!gpio_item[pin].read(val))
        return info.Env().Undefined();

    return Boolean::New(info.Env(), val > 0);
}

/**
 * gets gpio state as number.
 *
 * @param pin - gpio pin number 0..27
 * @returns 0/1, undefined on error
 */
Value get_gpio_num(const CallbackInfo &info)
{
    clear_error();
    CHECK_PARAM(1)
    uint32_t pin = info[0].ToNumber().Uint32Value();
    uint32_t val;

    if (pin >= N_PIN)
    {
        set_error(ERR_PIN);
        return Boolean::New(info.Env(), false);
    }

    if (!gpio_item[pin].read(val))
        return info.Env().Undefined();

    return Number::New(info.Env(), val);
}

/**
 * sets gpio state of output.
 *
 * @param pin - gpio pin number 0..27
 * @param mode - constant GPIO_MODE_..
 * @param value - state to set 0/1 or true/false
 * @returns true on ok, false on error
 */
Value set_gpio(const CallbackInfo &info)
{
    clear_error();
    CHECK_PARAM(2)
    uint32_t pin = info[0].ToNumber().Uint32Value();
    uint32_t val = info[1].ToNumber().Uint32Value();

    if (pin >= N_PIN)
    {
        set_error(ERR_PIN);
        return Boolean::New(info.Env(), false);
    }

    return Boolean::New(info.Env(), gpio_item[pin].write(val));
}

/**
 * toggle gpio output.
 *
 * @param pin - gpio pin number 0..27
 * @returns true on ok, false on error
 */
Value toggle_gpio(const CallbackInfo &info)
{
    clear_error();
    CHECK_PARAM(1)
    uint32_t pin = info[0].ToNumber().Uint32Value();

    if (pin >= N_PIN)
    {
        set_error(ERR_PIN);
        return Boolean::New(info.Env(), false);
    }

    return Boolean::New(info.Env(), gpio_item[pin].toggle());
}

/**
 * blink gpio output.
 *
 * @param pin - gpio pin number 0..27
 * @param period blink period in ms (1..), 0 stops blink
 * @returns true on ok, false on error
 */
Value blink_gpio(const CallbackInfo &info)
{
    clear_error();
    CHECK_PARAM(2)
    uint32_t pin    = info[0].ToNumber().Uint32Value();
    uint32_t period = info[1].ToNumber().Uint32Value();

    if (pin >= N_PIN)
    {
        set_error(ERR_PIN);
        return Boolean::New(info.Env(), false);
    }

    return Boolean::New(info.Env(), gpio_item[pin].blink(period));
}

/**
 * stop blink gpio output.
 *
 * @param pin - gpio pin number 0..27
 * @returns true on ok, false on error
 *
 * @note call of read/write/toggle/pulse/pwm functions stops also blink
 */
Value stop_blink_gpio(const CallbackInfo &info)
{
    clear_error();
    CHECK_PARAM(1)
    uint32_t pin = info[0].ToNumber().Uint32Value();

    if (pin >= N_PIN)
    {
        set_error(ERR_PIN);
        return Boolean::New(info.Env(), false);
    }

    gpio_item[pin].stopblink();

    return Boolean::New(info.Env(), true);
}

/**
 * pulse gpio output.
 *
 * @param pin - gpio pin number 0..27
 * @param period pulse period in ms (1..), 0 stops pulse
 * @returns true on ok, false on error
 */
Value pulse_gpio(const CallbackInfo &info)
{
    clear_error();
    CHECK_PARAM(2)
    uint32_t pin    = info[0].ToNumber().Uint32Value();
    uint32_t period = info[1].ToNumber().Uint32Value();

    if (pin >= N_PIN)
    {
        set_error(ERR_PIN);
        return Boolean::New(info.Env(), false);
    }

    return Boolean::New(info.Env(), gpio_item[pin].pulse(period));
}

/**
 * stop pulse gpio output.
 *
 * @param pin - gpio pin number 0..27
 * @returns true on ok, false on error
 *
 * @note call of read/write/toggle/blink/pwm functions stops also pulse
 */
Value stop_pulse_gpio(const CallbackInfo &info)
{
    clear_error();
    CHECK_PARAM(1)
    uint32_t pin = info[0].ToNumber().Uint32Value();

    if (pin >= N_PIN)
    {
        set_error(ERR_PIN);
        return Boolean::New(info.Env(), false);
    }

    gpio_item[pin].stoppulse();

    return Boolean::New(info.Env(), true);
}

/**
 * pwm gpio output.
 *
 * @param pin - gpio pin number 0..27
 * @param frequency pwm frequency in Hz (1..FREQ_MAX), 0 stops pwm
 * @param dutycycle pwm dutycycle in % (0..100), 0 turns output off, 100 turns output on
 * @returns true on ok, false on error
 */
Value pwm_gpio(const CallbackInfo &info)
{
    clear_error();
    CHECK_PARAM(3)
    uint32_t pin       = info[0].ToNumber().Uint32Value();
    uint32_t frequency = info[1].ToNumber().Uint32Value();
    uint32_t dutycycle = info[2].ToNumber().Uint32Value();

    if (pin >= N_PIN)
    {
        set_error(ERR_PIN);
        return Boolean::New(info.Env(), false);
    }

    return Boolean::New(info.Env(), gpio_item[pin].pwm(frequency, dutycycle));
}

/**
 * stop pwm gpio output.
 *
 * @param pin - gpio pin number 0..27
 * @returns true on ok, false on error
 *
 * @note call of read/write/toggle/blink/pulse functions stops also pwm
 */
Value stop_pwm_gpio(const CallbackInfo &info)
{
    clear_error();
    CHECK_PARAM(1)
    uint32_t pin = info[0].ToNumber().Uint32Value();

    if (pin >= N_PIN)
    {
        set_error(ERR_PIN);
        return Boolean::New(info.Env(), false);
    }

    gpio_item[pin].stoppwm();

    return Boolean::New(info.Env(), true);
}

/**
 * returns pwm frequency.
 * if pwm not started default frequency is returned (400Hz)
 *
 * @param pin - gpio pin number 0..27
 * @returns pwm frequency in Hz, undefined on error
 */
Value get_pwm_frequency(const CallbackInfo &info)
{
    clear_error();
    CHECK_PARAM(1)
    uint32_t pin = info[0].ToNumber().Uint32Value();

    if (pin >= N_PIN)
    {
        set_error(ERR_PIN);
        info.Env().Undefined();
    }

    return Number::New(info.Env(), gpio_item[pin].get_frequency());
}

/**
 * returns pwm duty cycle.
 * if pwm not started default duty cycle is returned (50%)
 *
 * @param pin - gpio pin number 0..27
 * @returns duty cycle in %, undefined on error
 */
Value get_pwm_dutycycle(const CallbackInfo &info)
{
    clear_error();
    CHECK_PARAM(1)
    uint32_t pin = info[0].ToNumber().Uint32Value();

    if (pin >= N_PIN)
    {
        set_error(ERR_PIN);
        info.Env().Undefined();
    }

    return Number::New(info.Env(), gpio_item[pin].get_dutycycle());
}

#define ADDFN(fn) exports.Set(#fn, Function::New(env, fn))
#define ADDNUM(num) exports.Set(#num, Number::New(env, num))

// export functions and constans
Object export_all(Env env, Object exports)
{
    ADDFN(version);
    ADDFN(error_text);

    ADDFN(deinit_gpio);
    ADDFN(init_gpio);
    ADDFN(get_gpio);
    ADDFN(get_gpio_num);
    ADDFN(set_gpio);
    ADDFN(toggle_gpio);

    ADDFN(blink_gpio);
    ADDFN(stop_blink_gpio);

    ADDFN(pulse_gpio);
    ADDFN(stop_pulse_gpio);

    ADDFN(pwm_gpio);
    ADDFN(stop_pwm_gpio);
    ADDFN(get_pwm_frequency);
    ADDFN(get_pwm_dutycycle);

    ADDNUM(GPIO_MODE_INPUT_NOPULL);
    ADDNUM(GPIO_MODE_INPUT_PULLDOWN);
    ADDNUM(GPIO_MODE_INPUT_PULLUP);
    ADDNUM(GPIO_MODE_OUTPUT);
    ADDNUM(GPIO_MODE_OUTPUT_SINK);
    ADDNUM(GPIO_MODE_OUTPUT_SOURCE);

    ADDNUM(DUTY_MIN);
    ADDNUM(DUTY_MAX);
    ADDNUM(FREQ_MIN);
    ADDNUM(FREQ_MAX);

    return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, export_all);



