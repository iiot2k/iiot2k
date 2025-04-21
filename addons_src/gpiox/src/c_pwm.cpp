/*
 * pwm handling
 *
 * (c) Derya Y. iiot2k@gmail.com
 *
 * c_pwm.cpp
 *
 */

#include <cmath>
#include <thread>
#include <mutex>
using namespace std;

#include "c_gpio.h"
#include "c_pwm.h"

#include "error_code.h"
#include "c_guard.h"
#include "gpio_def.h"

// #include <stdio.h>

c_pwm::c_pwm(c_gpio* gpio, uint32_t frequency, uint32_t dutycycle)
{
    m_gpio = gpio;
    set_data(frequency, dutycycle);

    // start thread
    thread th(&c_pwm::loop, this);
    th.detach();
}

c_pwm::~c_pwm()
{
    // stop thread
    m_timer.stop();
}

// calculate t_on t_off
void c_pwm::calc_on_off(uint32_t frequency, uint32_t dutycycle)
{
    if (frequency > FREQ_MAX)
        frequency = FREQ_MAX;
    else if (frequency < FREQ_MIN)
        frequency = FREQ_MIN;

    if (dutycycle > DUTY_MAX)
        dutycycle = DUTY_MAX;

    if (dutycycle == DUTY_MAX)
    {
        m_ton = 1000l;
        m_toff = 0l;
    }
    else if (dutycycle == DUTY_MIN)
    {
        m_ton = 0l;
        m_toff = 1000l;
    }
    else
    {
        double period = round(1000000.0/double(frequency));
        double ton_d = round(period * double(dutycycle) / 100.0);
        double toff_d = period - ton_d;

        m_ton = int64_t(ton_d);
        m_toff = int64_t(toff_d);
    }

    m_frequency = frequency;
    m_dutycycle = dutycycle;
}

// set pwm data
void c_pwm::set_data(uint32_t frequency, uint32_t dutycycle)
{
    c_guard guard(&m_mtx);
    calc_on_off(frequency, dutycycle);
    m_newdata.store(true);
}

// get pwm data
void c_pwm::get_data(int64_t& ton, int64_t& toff)
{
    c_guard guard(&m_mtx);
    ton = m_ton;
    toff = m_toff;
    m_newdata.store(false);
}

// pwm loop
void c_pwm::loop()
{
    int64_t ton;
    int64_t toff;

    while(1)
    {
        if (m_newdata.load())
            get_data(ton, toff);

        if ((ton > 0) && (toff > 0))
        {

            m_gpio->write(1);

            if (!m_timer.sleep_us(ton))
                break;

            m_gpio->write(0);

            if (!m_timer.sleep_us(toff))
                break;
        }
        else if (ton == 0)
        {
            m_gpio->write(0);  // 0%

            if (!m_timer.sleep_ms(1))
                break;
        }
        else if (toff == 0)
        {
            m_gpio->write(1);  // 100%

            if (!m_timer.sleep_ms(1))
                break;
        }
    }

    m_gpio->write(0);  // 0%

    // ack stop request
    m_timer.ack_stop();
}
