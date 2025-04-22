/*
 * gpio handler
 *
 * (c) Derya Y. iiot2k@gmail.com
 *
 * c_gpio.cpp
 *
 */

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <string.h>

#include "gpio.h"
#include "c_gpio.h"
#include "error_code.h"

//******* chip
// since kernel 6.6.45 all chip address is 0
#define CHIPNAME_CHIP0 "/dev/gpiochip0"
#define CHIPNAME_CHIP4 "/dev/gpiochip4"

class c_chip
{
public:
    c_chip()
    {
        // open chip4
        m_fd = open(CHIPNAME_CHIP4, O_RDWR | O_CLOEXEC);

        // open chip0 if chip4 not open
        if (m_fd == -1)
            m_fd = open(CHIPNAME_CHIP0, O_RDWR | O_CLOEXEC);
    };

    ~c_chip()
    {
        // close chip handle
        if (m_fd != -1)
            close(m_fd);
    };


    /**
     * @brief returns chip handle
     */
    inline int32_t get_fd() { return m_fd; }

private:
    int32_t m_fd;
};

static c_chip chip;

//******* gpio

c_gpio::c_gpio(uint32_t pin, bool input)
{
    m_fd = -1;

    // check if chip is open
    if (chip.get_fd() == -1)
    {
        set_error(ERR_CHIP);
        return;
    }

    gpio_v2_line_request line_request;

    memset(&line_request, 0, sizeof(line_request));

    line_request.num_lines = 1;
    line_request.offsets[0] = pin;

    if (input) // init to input, pullup (DOUT)
    {
        line_request.config.flags = GPIO_V2_LINE_FLAG_INPUT + GPIO_V2_LINE_FLAG_BIAS_PULL_UP;
        line_request.config.num_attrs = 1;
        line_request.config.attrs[0].mask = 1;
        line_request.config.attrs[0].attr.id = GPIO_V2_LINE_ATTR_ID_DEBOUNCE;
        line_request.config.attrs[0].attr.debounce_period_us = 0;
    }
    else // init to output (PD_SCK)
    {
        line_request.config.flags = GPIO_V2_LINE_FLAG_OUTPUT;
        line_request.config.num_attrs = 1;
        line_request.config.attrs[0].mask = 1;
        line_request.config.attrs[0].attr.id = GPIO_V2_LINE_ATTR_ID_OUTPUT_VALUES;
        line_request.config.attrs[0].attr.values = 0;
    }

    // set gpio pin
    if (ioctl(chip.get_fd(), GPIO_V2_GET_LINE_IOCTL, &line_request) == -1)
    {
        set_error(ERR_SYS);
        return;
    }

    // check for valid handle
    if (line_request.fd < 0)
    {
        set_error(ERR_SYS);
        return;
    }

    // set file handle
    m_fd = line_request.fd;
}

c_gpio::~c_gpio()
{
    // close pin handle if open
    if (m_fd != -1)
        close(m_fd);
}

uint8_t c_gpio::read()
{
    gpio_v2_line_values line_values;
    line_values.mask = 1;
    line_values.bits = 0;

    // read gpio pin
    if (ioctl(m_fd, GPIO_V2_LINE_GET_VALUES_IOCTL, &line_values) == -1)
        return set_error(ERR_SYS);

    return (line_values.bits == 1) ? 1 : 0;
}

bool c_gpio::write(uint32_t val)
{
    gpio_v2_line_values line_values;
    line_values.mask = 1;
    line_values.bits = val > 0 ? 1 : 0;

    // write gpio pin
    if (ioctl(m_fd, GPIO_V2_LINE_SET_VALUES_IOCTL, &line_values) == -1)
        return set_error(ERR_SYS);

    return true;
}
