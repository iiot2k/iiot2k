/*
 * pulse handling
 *
 * (c) Derya Y. iiot2k@gmail.com
 *
 * c_pulse.cpp
 *
 */

#include <cmath>
#include <thread>
#include <mutex>
using namespace std;

#include "c_gpio.h"
#include "c_pulse.h"

#include "error_code.h"
#include "c_guard.h"

// #include <stdio.h>

c_pulse::c_pulse(c_gpio* gpio, int32_t tpulse)
{
    m_gpio = gpio;
    m_tpulse.store(tpulse);

    // start thread
    thread th(&c_pulse::pulse, this);
    th.detach();
}

c_pulse::~c_pulse()
{
    // stop thread
    m_timer.stop();

    // turn output off
    m_gpio->write(0);
}

void c_pulse::pulse()
{
    m_gpio->write(1);

    m_timer.sleep_ms(m_tpulse.load());

    // turn output off
    m_gpio->write(0);

    // ack stop request
    m_timer.ack_stop();
}
