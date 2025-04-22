/*
 * hx711 sensor driver
 *
 * (c) Derya Y. iiot2k@gmail.com
 *
 * c_hx711.cpp
 *
 */

#include <cmath>
using namespace std;

#include "hx711_def.h"
#include "c_hx711.h"
#include "c_priority.h"
#include "error_code.h"

c_hx711::c_hx711()
{
    m_gpio_dt = NULL;
    m_gpio_cl = NULL;
    m_gain = GAIN_A128;
}

c_hx711::~c_hx711()
{
    if (m_gpio_cl != NULL)
    {
        // power down hx711
        m_gpio_cl->write(0);
        m_gpio_cl->write(1);

        delete (c_gpio*) m_gpio_cl;
    }

    if (m_gpio_dt != NULL)
        delete (c_gpio*) m_gpio_dt;
}

// init gpio and hx711
bool c_hx711::init(uint32_t pin_dt, uint32_t pin_cl)
{
    m_gpio_dt = new c_gpio(pin_dt, true); // input pullup
    m_gpio_cl = new c_gpio(pin_cl, false); // output

    if ((m_gpio_dt->get_fd() == -1) || (m_gpio_cl->get_fd() == -1))
        return set_error(ERR_SYS);

    // power up hx711
    m_gpio_cl->write(0);

    // wait hx711 ready
    return wait_ready();
}

// read hx711
bool c_hx711::read(uint32_t gain, double& value, uint32_t nread)
{
    value = 0.0;

    // if gain changed set gain and port with once call
    if (gain != m_gain)
    {
        if (!wait_ready())
            return false;

        read_raw(gain);
        m_gain = gain;
    }

    // adjust nread
    if (nread == 0)
        nread = 10;

    // read multiple times
    for (uint32_t i=0; i < nread; i++)
    {
        if (!wait_ready())
            return false;

        value += double(read_raw(gain));
    }

    // build average
    value = round(value / double(nread));

    return true;
}

// read hx711 raw data
int32_t c_hx711::read_raw(uint32_t gain)
{
    c_priority priority;

    uint32_t data = 0;

    // read from hx711
    for (int32_t i = 0; i < 24; i++)
    {
        m_gpio_cl->write(1);
        m_timer.delay_us(1);
        data = (data << 1) | m_gpio_dt->read();
        m_gpio_cl->write(0);
    }

    // set gain
    for (uint8_t i = 0; i < gain; i++)
    {
        m_gpio_cl->write(1);
        m_timer.delay_us(1);
        m_gpio_cl->write(0);
    }

    // convert to int32_t
    if (data & 0x800000)
        data |= 0xFF000000;

    return int32_t(data);
}

// waits hx711 ready until timeout
bool c_hx711::wait_ready()
{
    c_timer timer;

    // wait 600ms for ready
    for(uint32_t t=0; t < 60; t++)
    {
        if (m_gpio_dt->read() == 0)
            return true;

        timer.sleep_ms(10);
    }

    return set_error(ERR_TIMEOUT);
}


