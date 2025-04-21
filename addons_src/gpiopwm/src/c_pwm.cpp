/*
 * sensor handling
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

#include "pwm_def.h"
#include "c_pwm.h"

#include "error_code.h"
#include "c_guard.h"

// start pwm loop
c_pwmthread::c_pwmthread(c_gpio* gpio, int64_t ton, int64_t toff, bool realtime)
{
    m_gpio = gpio;
    m_ton = ton;
    m_toff = toff;
    m_realtime = realtime;

    // start thread
    thread th(&c_pwmthread::loop, this);
    th.detach();
}

// stop pwm loop
c_pwmthread::~c_pwmthread()
{
    // stop thread
    m_timer.stop();

    // turn output off
    m_gpio->write(0);
}

// set pwm data
void c_pwmthread::set_data(int64_t ton, int64_t toff, bool realtime)
{
    c_guard guard(&m_mtx);
    m_ton = ton;
    m_toff = toff;
    m_realtime = realtime;
}

// get pwm data
void c_pwmthread::get_data(int64_t& ton, int64_t& toff, bool& realtime)
{
    c_guard guard(&m_mtx);
    ton = m_ton;
    toff = m_toff;
    realtime = m_realtime;
}

// pwm main loop
void c_pwmthread::loop()
{
    int64_t ton;
    int64_t toff;
    bool realtime;

    while(1)
    {
        get_data(ton, toff, realtime);

        if ((ton > 0) && (toff > 0))
        {
            m_gpio->write(1);

            if (realtime)
            {
                if (!m_timer.delay_us(ton))
                    break;
            }
            else
            {
                if (!m_timer.sleep_us(ton))
                    break;
            }

            m_gpio->write(0);

            if (realtime)
            {
                if (!m_timer.delay_us(toff))
                    break;
            }
            else
            {
                if (!m_timer.sleep_us(toff))
                    break;
            }
        }
        else if (ton == 0)
        {
            m_gpio->write(0);  // 0%

            if (!m_timer.sleep_ms(1))
                break;
        }
        else if (m_toff == 0)
        {
            m_gpio->write(1);  // 100%

            if (!m_timer.sleep_ms(1))
                break;
        }
    }

    // ack stop request
    m_timer.ack_stop();
}

// init pwm
c_pwm::c_pwm(uint32_t pin, uint32_t frequency, uint32_t dutycycle, bool realtime)
{
    int64_t ton, toff;

    calc_on_off(frequency, dutycycle, ton, toff);

    m_gpio = new c_gpio(pin);

    if (isinit())
        m_pwmthread = new c_pwmthread(m_gpio, ton, toff, realtime);
    else
        m_pwmthread = NULL;
}

c_pwm::~c_pwm()
{
    if (m_pwmthread != NULL)
        delete (c_pwmthread*) m_pwmthread;

    if (m_gpio != NULL)
        delete (c_gpio*) m_gpio;
}

// update pwm
bool c_pwm::update(uint32_t frequency, uint32_t dutycycle, bool realtime)
{
    int64_t ton, toff;

    calc_on_off(frequency, dutycycle, ton, toff);

    if (!isinit())
        return false;

    if (m_pwmthread == NULL)
        return false;

    m_pwmthread->set_data(ton, toff, realtime);

    return true;
}

// calculate on off time
void c_pwm::calc_on_off(uint32_t frequency, uint32_t dutycycle, int64_t& ton, int64_t& toff)
{
    if (frequency > FREQ_MAX)
        frequency = FREQ_MAX;
    else if (frequency < FREQ_MIN)
        frequency = FREQ_MIN;

    if (dutycycle > DUTY_MAX)
        dutycycle = DUTY_MAX;

    if (dutycycle == DUTY_MAX)
    {
        ton = 1000;
        toff = 0;
    }
    else if (dutycycle == DUTY_MIN)
    {
        ton = 0;
        toff = 1000;
    }
    else
    {
        double period = round(1000000.0/double(frequency));
        double ton_d = round(period * double(dutycycle) / 100.0);
        double toff_d = period - ton_d;

        ton = int64_t(ton_d);
        toff = int64_t(toff_d);
    }

    m_frequency = frequency;
    m_dutycycle = dutycycle;
}
