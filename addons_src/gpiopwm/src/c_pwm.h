/*
 * pwm handling
 *
 * (c) Derya Y. iiot2k@gmail.com
 *
 * c_pwm.h
 *
 */

#pragma once

#include <mutex>
using namespace std;

#include "c_gpio.h"
#include "error_code.h"
#include "c_timer.h"

class c_pwmthread
{
public:
    c_pwmthread(c_gpio* gpio, int64_t ton, int64_t toff, bool realtime);
    ~c_pwmthread();

    // set pwm data
    void set_data(int64_t ton, int64_t toff, bool realtime);

    // get pwm data
    void get_data(int64_t& ton, int64_t& toff, bool& realtime);

private:
    c_gpio* m_gpio;
    c_timer m_timer;

    int64_t m_ton; // pwn on time in us
    int64_t m_toff; // pwm off time in us
    bool m_realtime; // pwm in realtime

    mutex m_mtx;

    // pwm loop
    void loop();
};

class c_pwm
{
public:
    // init pwm gpio
    c_pwm(uint32_t pin, uint32_t frequency, uint32_t dutycycle, bool realtime);

    // close pwm gpio
    ~c_pwm();

    // check if pwm gpio is valid
    // return false on not valid
    bool isinit() { return (m_gpio != NULL) ? (m_gpio->get_fd() != -1) : false; }

    // update running pwm
    bool update(uint32_t frequency, uint32_t dutycycle, bool realtime);

    // get pwm frequency
    inline uint32_t get_frequency() { return m_frequency; };

    // get pwm dutycycle
    inline uint32_t get_dutycycle() { return m_dutycycle; };

private:
    // calculate on off time
    void calc_on_off(uint32_t frequency, uint32_t dutycycle, int64_t& ton, int64_t& toff);

    uint32_t m_frequency;
    uint32_t m_dutycycle;

    c_gpio* m_gpio; // pwm gpio
    c_pwmthread *m_pwmthread; // pwm tread
};

class c_pwm_item
{
public:
    c_pwm_item()
    {
        m_pwm = NULL;
    }

    ~c_pwm_item()
    {
        deinit();
    }

    // deinit pwm item
    void deinit()
    {
        if (m_pwm != NULL)
        {
            delete (c_pwm*) m_pwm;
            m_pwm = NULL;
        }
    }

    // init pwm item or if run update pwm item
    bool init(uint32_t pin, uint32_t frequency, uint32_t dutycycle, bool realtime)
    {
        if (m_pwm == NULL)
        {
            m_pwm = new c_pwm(pin, frequency, dutycycle, realtime);

            if (m_pwm == NULL)
                return set_error(ERR_SYS);
            else if (!m_pwm->isinit())
                return set_error(ERR_SYS);
        }
        else if (!m_pwm->update(frequency, dutycycle, realtime))
            return set_error(ERR_SYS);

        return true;
    }

    c_pwm* m_pwm;
};
