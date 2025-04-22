/*
 * hx711 driver
 *
 * (c) Derya Y. iiot2k@gmail.com
 *
 * c_hx711.h
 *
 */

#pragma once

#include <mutex>
using namespace std;

#include "c_gpio.h"
#include "c_timer.h"
#include "error_code.h"
#include "c_guard.h"

class c_hx711
{
public:
    // init hx711
    c_hx711();

    // close hx711
    ~c_hx711();

    // init hx711
    bool init(uint32_t pin_dt, uint32_t pin_cl);

    // read hx711
    bool read(uint32_t gain, double& value, uint32_t nread);

private:
    // waits hx711 ready until timeout
    bool wait_ready();

    // read hx711 raw data
    int32_t read_raw(uint32_t gain);

    uint32_t m_gain; // save gain

    c_gpio* m_gpio_dt; // hx711 data gpio
    c_gpio* m_gpio_cl; // hx711 clock gpio
    c_timer m_timer; // used for delay and sleep
};

class c_hx711_item
{
public:
    c_hx711_item()
    {
        m_hx711 = NULL;
    }

    ~c_hx711_item()
    {
        deinit();
    }

    void deinit()
    {
        if (m_hx711 != NULL)
            delete (c_hx711*) m_hx711;

        m_hx711 = NULL;
    }

    // init gpio and hx711
    bool init(uint32_t pin_dt, uint32_t pin_cl)
    {
        c_guard guard(&m_mtx);

        if (m_hx711 == NULL)
        {
            m_hx711 = new c_hx711();

            if (!m_hx711->init(pin_dt, pin_cl))
            {
                deinit();
                return false;
            }
        }
        else
            return set_error(ERR_ISINIT);

        return true;
    }

    // read hx711
    bool read(uint32_t gain, double& value, uint32_t nread)
    {
        c_guard guard(&m_mtx);

        if (m_hx711 == NULL)
            return set_error(ERR_ISNOINIT);

        return m_hx711->read(gain, value, nread);
    }

    // check if init
    bool isinit()
    {
        return (m_hx711 != NULL);
    }

private:
    c_hx711* m_hx711;
    mutex m_mtx; // mutex for threadsafe
};
