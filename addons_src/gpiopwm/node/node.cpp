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

#include "c_pwm.h"
#include "pwm_def.h"
#include "exception.h"
#include "error_code.h"

#define MODULE_VERSION "3.0.0"

#define OUTTEXT "@iiot2k/swpwm version: " MODULE_VERSION

// print banner
__attribute__((constructor)) static void libstart()
{
    puts(OUTTEXT);
}

// storage for gpio pwm
c_pwm_item pwm_item[N_PIN];

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
 * stops pwm and deinits gpio pin.
 *
 * @param pin - gpio pin number 0..27
 * @returns true on ok, false on error
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

    pwm_item[pin].deinit();

    return Boolean::New(info.Env(), true);
}

/**
 * returns actual pwm frequency.
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
        return info.Env().Undefined();
    }

    if (pwm_item[pin].m_pwm == NULL)
    {
        set_error(ERR_ISNOINIT);
        return info.Env().Undefined();
    }

    return Number::New(info.Env(), pwm_item[pin].m_pwm->get_frequency());
}

/**
 * returns actual pwm duty cycle.
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
        return info.Env().Undefined();
    }

    if (pwm_item[pin].m_pwm == NULL)
    {
        set_error(ERR_ISNOINIT);
        return info.Env().Undefined();
    }

    return Number::New(info.Env(), pwm_item[pin].m_pwm->get_dutycycle());
}

/**
 * sets pwm data, starts pwm on first call.
 *
 * @param pin - gpio pin number 0..27
 * @param frequency - in Hz 1..45000
 * @param dutycycle - in % 0..100
 * @param realtime - true: generate pwm in realtime
 * @returns true on ok, false on error
 */
Value set_pwm(const CallbackInfo &info)
{
    clear_error();
    CHECK_PARAM(4)
    uint32_t pin       = info[0].ToNumber().Uint32Value();
    uint32_t frequency = info[1].ToNumber().Uint32Value();
    uint32_t dutycycle = info[2].ToNumber().Uint32Value();
    bool realtime      = info[3].ToBoolean().Value();

    if (pin >= N_PIN)
    {
        set_error(ERR_PIN);
        return Boolean::New(info.Env(), false);
    }

    return Boolean::New(info.Env(), pwm_item[pin].init(pin, frequency, dutycycle, realtime));
}

#define ADDFN(fn) exports.Set(#fn, Function::New(env, fn))
#define ADDNUM(num) exports.Set(#num, Number::New(env, num))

// export functions and constans
Object export_all(Env env, Object exports)
{
    ADDFN(version);
    ADDFN(error_text);

    ADDFN(deinit_gpio);
    ADDFN(get_pwm_frequency);
    ADDFN(get_pwm_dutycycle);
    ADDFN(set_pwm);

    ADDNUM(DUTY_MIN);
    ADDNUM(DUTY_MAX);
    ADDNUM(FREQ_MIN);
    ADDNUM(FREQ_MAX);

    return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, export_all);



