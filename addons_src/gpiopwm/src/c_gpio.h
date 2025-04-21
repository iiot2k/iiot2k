/*
 * gpio handler
 *
 * (c) Derya Y. iiot2k@gmail.com
 *
 * c_gpio.h
 *
 */

#pragma once
#include <stdint.h>

class c_gpio
{
public:
    /**
     * @brief inits gpio pin
     * @param pin gpio pin
     * @note on error m_fd is -1 and error is set
     */
    c_gpio(uint32_t pin);

    /**
     * @brief deinits gpio pin
     */
    ~c_gpio();

    /**
     * @brief returns pin handle
     * @returns pin handle, -1 if not open
     */
    int32_t get_fd() { return m_fd; }

    /**
     * @brief sets pin state
     * @param val state to write 0 or 1
     * @returns false: error, true: ok
     */
    bool write(uint32_t val);

private:
    int32_t m_fd; // gpio pin handle
};
