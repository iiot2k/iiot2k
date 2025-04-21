/*
 * pwm handling
 *
 * (c) Derya Y. iiot2k@gmail.com
 *
 * c_pwm.h
 *
 */

#pragma once

#include <atomic>
#include <mutex>
using namespace std;

#include "error_code.h"
#include "c_timer.h"

class c_gpio;

class c_pwm
{
public:
    c_pwm(c_gpio* gpio, uint32_t frequency, uint32_t dutycycle);
    ~c_pwm();

    // set pwm data
    void set_data(uint32_t frequency, uint32_t dutycycle);

    inline uint32_t get_frequency() { return m_frequency; };
    inline uint32_t get_dutycycle() { return m_dutycycle; };

private:
    // get pwm data
    void get_data(int64_t& ton, int64_t& toff);

    // calculate t_on t_off
    void calc_on_off(uint32_t frequency, uint32_t dutycycle);

    c_gpio* m_gpio;
    c_timer m_timer;

    uint32_t m_frequency;
    uint32_t m_dutycycle;

    int64_t m_ton; // pwn on time in us
    int64_t m_toff; // pwm off time in us

    atomic_bool m_newdata;

    mutex m_mtx;

    void loop();
};

