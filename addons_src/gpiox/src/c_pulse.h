/*
 * pulse handling
 *
 * (c) Derya Y. iiot2k@gmail.com
 *
 * c_pulse.h
 *
 */

#pragma once

#include <atomic>
using namespace std;

#include "error_code.h"
#include "c_timer.h"

class c_gpio;

class c_pulse
{
public:
    c_pulse(c_gpio* gpio, int32_t tpulse);
    ~c_pulse();

private:
    c_gpio* m_gpio;
    c_timer m_timer;

    atomic_int32_t m_tpulse; // pulse period ms

    void pulse();
};

