/*
 * blink handling
 *
 * (c) Derya Y. iiot2k@gmail.com
 *
 * c_blink.cpp
 *
 */

#include <cmath>
#include <thread>
#include <mutex>
using namespace std;

#include "c_gpio.h"
#include "c_blink.h"

#include "error_code.h"
#include "c_guard.h"

c_blink::c_blink(c_gpio* gpio, int32_t tblink)
{
    m_gpio = gpio;
    m_tblink.store(tblink);

    // start thread
    thread th(&c_blink::loop, this);
    th.detach();
}

c_blink::~c_blink()
{
    // stop thread
    m_timer.stop();

    // turn output off
    m_gpio->write(0);
}

void c_blink::set_tblink(int32_t tblink)
{
    m_tblink.store(tblink);
}

void c_blink::loop()
{
    uint32_t outval = 1;

    while(1)
    {
        m_gpio->write(outval);

        outval = (outval == 1) ? 0 : 1;

        if (!m_timer.sleep_ms(m_tblink.load()))
            break;
    }

    // ack stop request
    m_timer.ack_stop();
}
