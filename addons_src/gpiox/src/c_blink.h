/*
 * blink handling
 *
 * (c) Derya Y. iiot2k@gmail.com
 *
 * c_blink.h
 *
 */

#pragma once

#include <atomic>
using namespace std;

#include "error_code.h"
#include "c_timer.h"

class c_gpio;

class c_blink
{
public:
    c_blink(c_gpio* gpio, int32_t tblink);
    ~c_blink();

    // set tblink
    void set_tblink(int32_t tblink);

private:
    c_gpio* m_gpio;
    c_timer m_timer;

    atomic_int32_t m_tblink; // blink period ms

    void loop();
};

