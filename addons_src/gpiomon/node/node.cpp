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

#include "gpiomon_def.h"
#include "c_gpio.h"
#include "c_worker.h"
#include "c_tsqueue.h"
#include "exception.h"
#include "error_code.h"

#define MODULE_VERSION "3.0.2"

#define OUTTEXT "@iiot2k/gpiomon version: " MODULE_VERSION

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
 * stops monitor and deinits gpio pin.
 *
 * @param pin - gpio pin number 0..27
 * @returns true on ok, false on error
 *
 * @note GPIO pin is deinitialized when the program ends
 * @note stops also monitor
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

// gpio watch thread
class c_gpio_watch : public c_worker
{
public:
	c_gpio_watch(c_gpio* gpio, c_tsqueue<s_gpio_event*>* ptsqueue)
	{
		m_gpio = gpio;
		m_ptsqueue = ptsqueue;
	}

	~c_gpio_watch() {}

	void Execute()
    {
		s_gpio_event gpio_event;

		// wait until gpio changes or stop request
        while(m_gpio->watch(gpio_event))
        {
        	s_gpio_event* pevent = new s_gpio_event;

        	if (pevent == NULL)
        	{
        		break;
        	}

			pevent->pin = gpio_event.pin;
			pevent->state = gpio_event.state;
			pevent->edge = gpio_event.edge;

			// add event to queue
			m_ptsqueue->Push(pevent);
        }

        // inform main watch thread for stop
		m_ptsqueue->Push(NULL);
    }

private:
    c_gpio* m_gpio;
    c_tsqueue<s_gpio_event*>* m_ptsqueue;
};

// main watch thread
class c_gpio_watch_main : public c_worker
{
public:
	c_gpio_watch_main(Env& env, Function& cb, c_gpio* gpio)
    {
		m_gpio = gpio;
        m_threadSafeCallback = ThreadSafeFunction::New(env, cb, "Watch CB", 0, 1);
    }

    ~c_gpio_watch_main() {}

    void Execute()
    {
        auto callback = [](Env env, Function jsCallback, s_gpio_event* event)
        {
              jsCallback.Call({
            	  Number::New(env, event->pin),
                  Number::New(env, event->state),
            	  Number::New(env, event->edge)
              });

              delete (s_gpio_event*) event;
        };

        // start gpio watch thread
		c_gpio_watch* wk = new c_gpio_watch(m_gpio, &m_tsqueue);
        wk->Queue();

        while(1)
        {
        	// wait until event on queue
            s_gpio_event* pevent = m_tsqueue.Pop();

            // is stop signal ?
        	if (pevent == NULL)
        		break;

        	// call callback
			m_threadSafeCallback.NonBlockingCall(pevent, callback);
        }

        m_threadSafeCallback.Release();
        m_gpio->ack_stop();
    }

private:
    c_gpio* m_gpio;
    c_tsqueue<s_gpio_event*> m_tsqueue;
    ThreadSafeFunction m_threadSafeCallback;
};

/**
 * inits gpio and starts monitor.
 *
 * @param pin - gpio pin number 0..27
 * @param mode - constant GPIO_MODE_INPUT_..
 * @param debounce - time in us, 0 for no debounce
 * @param edge - constant GPIO_EDGE_..
 * @param callback - call that receives gpio event,
 * pin: gpio pin number 0..27,
 * state: gpio input state (0/1),
 * edge: GPIO_EDGE_RISING or GPIO_EDGE_FALLING
 * @returns true on ok, false on error
 */
Value monitor_gpio(const CallbackInfo &info)
{
    clear_error();
    CHECK_PARAM(5)
    uint32_t pin      = info[0].ToNumber().Uint32Value();
    uint32_t mode     = info[1].ToNumber().Uint32Value();
    uint32_t debounce = info[2].ToNumber().Uint32Value();
    uint32_t edge     = info[3].ToNumber().Uint32Value();
    Function cb       = info[4].As<Function>();

    Env env = info.Env();

    if (pin >= N_PIN)
    {
        set_error(ERR_PIN);
        return Boolean::New(info.Env(), false);
    }

    // check if gpio already init
    if (gpio_item[pin].isinit())
    {
    	set_error(ERR_ISINIT);
        return Boolean::New(info.Env(), false);
    }

    // init gpio
    if (!gpio_item[pin].init(pin, mode, debounce, edge))
        return Boolean::New(info.Env(), false);

    // start main watch thread
    c_gpio_watch_main* wk = new c_gpio_watch_main(env, cb, gpio_item[pin].m_gpio);
    wk->Queue();

    return Boolean::New(info.Env(), true);
}

/**
 * reads gpio input state.
 *
 * @param pin - gpio pin number 0..27
 * @returns gpio input state (0/1), undefined on error
 * @note monitor_gpio must called before
 */
Value read_gpio(const CallbackInfo &info)
{
    clear_error();
    CHECK_PARAM(1)
    uint32_t pin = info[0].ToNumber().Uint32Value();

    Env env = info.Env();

    if (pin >= N_PIN)
    {
        set_error(ERR_PIN);
        return env.Undefined();
    }

    // check if gpio init
    if (!gpio_item[pin].isinit())
    {
    	set_error(ERR_ISNOINIT);
        return env.Undefined();
    }

    uint32_t val;

    // read gpio
    if (!gpio_item[pin].read(val))
        return env.Undefined();

    return Number::New(info.Env(), val);
}

#define ADDFN(fn) exports.Set(#fn, Function::New(env, fn))
#define ADDNUM(num) exports.Set(#num, Number::New(env, num))

// export functions and constans
Object export_all(Env env, Object exports)
{
    ADDFN(version);
    ADDFN(error_text);

    ADDFN(deinit_gpio);
    ADDFN(monitor_gpio);
    ADDFN(read_gpio);

    ADDNUM(GPIO_MODE_INPUT_NOPULL);
    ADDNUM(GPIO_MODE_INPUT_PULLDOWN);
    ADDNUM(GPIO_MODE_INPUT_PULLUP);

    ADDNUM(GPIO_EDGE_RISING);
    ADDNUM(GPIO_EDGE_FALLING);
    ADDNUM(GPIO_EDGE_BOTH);

    return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, export_all);



