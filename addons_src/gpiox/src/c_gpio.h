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

#include "c_blink.h"
#include "c_pulse.h"
#include "c_pwm.h"

#include "error_code.h"

class c_gpio
{
public:
    c_gpio();

    /**
     * @brief deinits gpio pin
     */
    ~c_gpio();

    /**
     * @brief inits gpio pin
     * @param pin gpio pin
     * @param mode constant GPIO_MODE_..
     * @param setval input: debounce time in ms, output gpio state
     * @returns true: ok, false: error
     */
    bool init(uint32_t pin, uint32_t mode, uint32_t setval);

    /**
     * @brief returns pin handle
     * @returns pin handle, -1 if not open
     */
    inline int32_t get_fd() { return m_fd; }

    /**
     * @brief check if pin is output
     * @returns true: output, false: input
     */
    inline bool isoutput() { return m_output; }

    /**
     * @brief reads gpio
     * @param val state to store
     * @returns true: ok, false: error
     */
    bool read(uint32_t& val);

    /**
     * @brief sets pin state
     * @param val state to write 0 or 1
     * @returns false: error, true: ok
     */
    bool write(uint32_t val);

    /**
     * @brief toggle output
     * @returns false: error, true: ok
     */
    bool toggle();

private:
    int32_t m_fd; // gpio pin handle
    bool m_output; // output flag
    mutex m_mtx; // lock mutex
};

class c_gpioitem
{
public:
    c_gpioitem();

    ~c_gpioitem();

    /**
     * @brief deinits gpio
     */
    void deinit();

    /**
     * @brief inits gpio pin
     * @param pin gpio pin
     * @param mode constant GPIO_MODE_..
     * @param setval input: debounce time in ms, output gpio state
     * @return true: ok, false: error
     */
    bool init(uint32_t pin, uint32_t mode, uint32_t setval);

    /**
     * @brief reads gpio
     * @param val state to store
     * @returns true: ok, false: error
     */
    bool read(uint32_t& val);

    /**
     * @brief sets pin state
     * @param val state to write 0 or 1
     * @returns false: error, true: ok
     */
    bool write(uint32_t val);

    /**
     * @brief toggle output
     * @returns false: error, true: ok
     */
    bool toggle();

    /**
     * @brief blinks output
     * @returns false: error, true: ok
     */
    bool blink(uint32_t tblink);

    /**
     * @brief pulse output
     * @returns false: error, true: ok
     */
    bool pulse(uint32_t tpulse);

    /**
     * @brief pwm output
     * @returns false: error, true: ok
     */
    bool pwm(uint32_t frequency, uint32_t dutycycle);

    /**
     * @brief get pwm frequency
     * @return pwm frequency or default
     */
    uint32_t get_frequency();

    /**
     * @brief get pwm duty cycle
     * @return pwm duty cycle or default
     */
    uint32_t get_dutycycle();

    /**
     * @brief stops blink if started
     */
    void stopblink();

    /**
     * @brief stops pulse
     */
    void stoppulse();

    /**
     * @brief stops pwm and set gpio
     */
    void stoppwm();

    /**
     * @brief check if gpio is init
     * @return true: is init, false: is not init
     */
    inline bool isinit() { return (m_gpio != NULL); }

    /**
     * @brief check if blink startet
     * @return true: started, false: not started
     */
    inline bool isblink() { return (m_blink != NULL); }

    /**
     * @brief check if pwm startet
     * @return true: started, false: not started
     */
    inline bool ispwm() { return (m_pwm != NULL); }

    /**
     * @brief check if pin is output
     * @return true: is output, false: is not output
     */
    inline bool isoutput() { return m_gpio->isoutput(); }

private:
    c_gpio* m_gpio;
    c_blink* m_blink; // thread for blink
    c_pulse* m_pulse; // thread for pulse
    c_pwm* m_pwm; // thread for pwm
};
