/*
 * gpio handler
 *
 * (c) Derya Y. iiot2k@gmail.com
 *
 * c_gpio.h
 *
 */

#pragma once

#include <mutex>
using namespace std;

#include <stdint.h>

#include "c_event.h"
#include "error_code.h"

// event data
struct s_gpio_event
{
    uint32_t pin;
    uint32_t state;
    uint32_t edge;
};

class c_gpio
{
public:
    /**
     * @brief constuctor
     */
    c_gpio(uint32_t pin);

    /**
     * @brief deinits gpio pin
     */
    ~c_gpio();

    /**
     * @brief inits gpio pin
     * @param mode const GPIO_MODE_INPUT_..
     * @param debounce time in ms
     * @param edge const GPIO_EDGE_..
     */
    bool init_gpio(uint32_t mode, uint32_t debounce, uint32_t edge);

    // acknowledge stop
    inline void ack_stop() { m_event.ack_stop(); }

    /**
     * @brief reads gpio
     * @param val state to store
     * @returns true: ok, false: error
     */
    bool read(uint32_t& val);

    /**
     * @brief watch gpio
     * @returns true: event occurs, false: stop event or error
     */
    bool watch(s_gpio_event& gpio_event);

private:
    uint32_t m_pin;
    int32_t m_fd; // gpio pin handle

    c_event m_event;
};

class c_gpioitem
{
public:
	c_gpioitem()
    {
		m_gpio = NULL;
	    m_mode = 0;
	    m_debounce = 0;
	    m_edge = 0;
    }

    ~c_gpioitem()
    {
    	deinit();
    }

    // deinit gpio
    void deinit()
    {
        if (m_gpio != NULL)
        {
            delete (c_gpio*) m_gpio;
            m_gpio = NULL;
        }
    }

    // init gpio
    bool init(uint32_t pin, uint32_t mode,  uint32_t debounce, uint32_t edge)
    {
        if (m_gpio != NULL)
        {
        	// check if data changed
            if ((m_mode != mode) || (m_debounce != debounce) || (m_edge != edge))
            	deinit();
            else
            	return set_error(ERR_ISINIT);
        }

        m_gpio = new c_gpio(pin);

		if (m_gpio == NULL)
			return set_error(ERR_SYS);
		else if (!m_gpio->init_gpio(mode, debounce, edge))
			return false;

		m_mode = mode;
	    m_debounce = debounce;
	    m_edge = edge;

	    return true;
    }

    // read gpio state
    bool read(uint32_t& val)
    {
        if (m_gpio == NULL)
        	return false;

        return m_gpio->read(val);
    }

    // check if gpio init
    bool isinit()
    {
    	return (m_gpio != NULL);
    }

    uint32_t m_mode;
    uint32_t m_debounce;
    uint32_t m_edge;

    c_gpio* m_gpio;
};
