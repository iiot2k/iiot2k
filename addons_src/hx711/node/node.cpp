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

#include "c_hx711.h"
#include "hx711_def.h"
#include "exception.h"
#include "error_code.h"
#include "c_worker.h"

#define MODULE_VERSION "3.0.0"

#define OUTTEXT "@iiot2k/hx711 version: " MODULE_VERSION

__attribute__((constructor)) static void libstart()
{
    puts(OUTTEXT);
}

/**
 * returns addon version.
 * @returns version text
 */
Value version(const CallbackInfo &info)
{
    return String::New(info.Env(), MODULE_VERSION);
}

/**
 * returns error text on failure.
 * @returns error text
 */
Value error_text(const CallbackInfo &info)
{
    return String::New(info.Env(), get_error_text());
}

// pin
#define N_PIN 28

// storage for hx711 items
c_hx711_item hx711_item[N_PIN];

/**
 * inits gpio for hx711 pins.
 *
 * @param pin_dt - gpio pin for hx711 DOUT pin (0..27)
 * @param pin_cl - gpio pin for hx711 PD_SCK pin (0..27)
 * @returns true on ok, false on error
 *
 */
Value init_gpio(const CallbackInfo &info)
{
    clear_error();
    CHECK_PARAM(2);

    uint32_t pin_dt = info[0].ToNumber().Uint32Value();
    uint32_t pin_cl = info[1].ToNumber().Uint32Value();

    // check pins
    if ((pin_dt >= N_PIN) || (pin_cl >= N_PIN) || (pin_dt == pin_cl))
    {
        set_error(ERR_PIN);
        return Boolean::New(info.Env(), false);
    }

    // check if pins already init
    if (hx711_item[pin_dt].isinit() || hx711_item[pin_cl].isinit())
    {
        set_error(ERR_ISINIT);
        return Boolean::New(info.Env(), false);
    }

    // init hx711
    if (!hx711_item[pin_dt].init(pin_dt, pin_cl))
        return Boolean::New(info.Env(), false);

    return Boolean::New(info.Env(), true);
}

/**
 * deinits gpio pin_dt and pin_dt.
 *
 * @param pin_dt - gpio pin number 0..27
 * @returns true on ok, false on error
 *
 * @note GPIO pins are deinitialized when the program ends
 */
Value deinit_gpio(const CallbackInfo &info)
{
    clear_error();
    CHECK_PARAM(1);
    uint32_t pin_dt = info[0].ToNumber().Uint32Value();

    // check pins
    if (pin_dt >= N_PIN)
    {
        set_error(ERR_PIN);
        return Boolean::New(info.Env(), false);
    }

    // deinits pins
    hx711_item[pin_dt].deinit();

    return Boolean::New(info.Env(), true);
}

// async read thread
class c_worker_read : public c_worker
{
    public:
        c_worker_read(Env& env, Function& cb, c_hx711_item* hx711_item, uint32_t gain, uint32_t nread)
        {
            m_hx711_item = hx711_item;
            m_gain = gain;
            m_nread = nread;

            m_threadSafeCallback = ThreadSafeFunction::New(env, cb, "worker_read", 0, 1);
        }

        ~c_worker_read() {}

        void Execute() override
        {
            auto callback = [](Env env, Function jsCallback, double* pvalue)
            {
                if (pvalue == NULL)
                    jsCallback.Call({ env.Undefined() });
                else
                {
                    jsCallback.Call({ Number::New(env, *pvalue) });

                    delete (double*) pvalue;
                }
            };

            double* pvalue = NULL;
            double value;

            if (m_hx711_item->read(m_gain, value, m_nread))
            {
                pvalue = new double;
                *pvalue = value;
            }

            m_threadSafeCallback.NonBlockingCall(pvalue, callback);
            m_threadSafeCallback.Release();
        }

    private:
        c_hx711_item* m_hx711_item;
        uint32_t m_gain;
        uint32_t m_nread;

        ThreadSafeFunction m_threadSafeCallback;
};

/**
 * @brief reads hx711 adc
 * @param pin_dt - gpio pin number 0..27
 * @param gain constant GAIN_..
 * @param nread count of reads for calculate average (1..)
 * @param callback asynchron call that receives adc value, undefined on error
 * @returns hx711 adc value, undefined on error or asynchron call
 *
 * callback parameter is optional, if omitted call is synchron
 */
Value read_adc(const CallbackInfo &info)
{
    clear_error();
    if (info.Length() == 3) { CHECK_PARAM(3); }
    else { CHECK_PARAM(4); }

    uint32_t pin_dt = info[0].ToNumber().Uint32Value();
    uint32_t gain   = info[1].ToNumber().Uint32Value();
    uint32_t nread  = info[2].ToNumber().Uint32Value();

    // check pins
    if (pin_dt >= N_PIN)
    {
        set_error(ERR_PIN);
        return info.Env().Undefined();
    }

    // check if pins not init
    if (!hx711_item[pin_dt].isinit())
    {
        set_error(ERR_ISNOINIT);
        return info.Env().Undefined();
    }

    // check gain
    if ((gain < GAIN_A128) || (gain > GAIN_A64))
    {
        set_error(ERR_PARAMETER);
        return info.Env().Undefined();
    }

    // adjust nread
    if (nread == 0)
        nread = 10;

    // callback ?
    if (info.Length() == 4)
    {
        Function cb = info[3].As<Function>();
        Env env = info.Env();
        c_worker_read* wk = new c_worker_read(env, cb, &hx711_item[pin_dt], gain, nread);
        wk->Queue();

        return info.Env().Undefined();
    }

    double value;

    if (!hx711_item[pin_dt].read(gain, value, nread))
        return info.Env().Undefined();

    return Number::New(info.Env(), value);
}

#define ADDFN(fn) exports.Set(#fn, Function::New(env, fn))
#define ADDNUM(num) exports.Set(#num, Number::New(env, num))

// exports functions and contans
Object export_all(Env env, Object exports)
{
    ADDFN(version);
    ADDFN(error_text);

    ADDFN(init_gpio);
    ADDFN(deinit_gpio);
    ADDFN(read_adc);

    ADDNUM(GAIN_A128); // channel A gain 128
    ADDNUM(GAIN_B32);  // channel B gain 32
    ADDNUM(GAIN_A64);  // channel A gain 64

    return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, export_all);



