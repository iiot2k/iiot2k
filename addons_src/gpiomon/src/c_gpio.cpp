/*
 * gpio handler
 *
 * (c) Derya Y. iiot2k@gmail.com
 *
 * c_gpio.cpp
 *
 */

#include <thread>
using namespace std;

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <string.h>

#include "gpio.h"
#include "c_gpio.h"
#include "error_code.h"
#include "c_guard.h"
#include "gpiomon_def.h"

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
c_gpio::c_gpio(uint32_t pin)
{
    m_fd = -1;
    m_pin = pin;
}

bool c_gpio::init_gpio(uint32_t mode, uint32_t debounce, uint32_t edge)
{
    // check if chip is open
    if (chip.get_fd() == -1)
        return set_error(ERR_CHIP);

    gpio_v2_line_request line_request;

    memset(&line_request, 0, sizeof(line_request));

    line_request.num_lines = 1;
    line_request.offsets[0] = m_pin;

    // set gpio parameter
    switch(mode)
    {
	case GPIO_MODE_INPUT_NOPULL:
		line_request.config.flags = GPIO_V2_LINE_FLAG_INPUT + GPIO_V2_LINE_FLAG_BIAS_DISABLED;
        break;

    default:
    case GPIO_MODE_INPUT_PULLDOWN:
    	line_request.config.flags = GPIO_V2_LINE_FLAG_INPUT + GPIO_V2_LINE_FLAG_BIAS_PULL_DOWN;
        break;

    case GPIO_MODE_INPUT_PULLUP:
    	line_request.config.flags = GPIO_V2_LINE_FLAG_INPUT + GPIO_V2_LINE_FLAG_BIAS_PULL_UP + GPIO_V2_LINE_FLAG_ACTIVE_LOW;
        break;
    }


    // set debounce
    line_request.config.num_attrs = 1;
    line_request.config.attrs[0].mask = 1;
    line_request.config.attrs[0].attr.id = GPIO_V2_LINE_ATTR_ID_DEBOUNCE;
    line_request.config.attrs[0].attr.debounce_period_us = debounce;

    // set edge parameter
    switch(edge)
    {
    case GPIO_EDGE_RISING:
    	line_request.config.flags += GPIO_V2_LINE_FLAG_EDGE_RISING;
        break;
    case GPIO_EDGE_FALLING:
    	line_request.config.flags += GPIO_V2_LINE_FLAG_EDGE_FALLING;
        break;
    default:
    case GPIO_EDGE_BOTH:
    	line_request.config.flags += GPIO_V2_LINE_FLAG_EDGE_RISING + GPIO_V2_LINE_FLAG_EDGE_FALLING;
        break;
    }

    // init gpio pin
    if (ioctl(chip.get_fd(), GPIO_V2_GET_LINE_IOCTL, &line_request) == -1)
        return set_error(ERR_SYS);

    // check for valid handle
    if (line_request.fd < 0)
        return set_error(ERR_SYS);

    // set file handle
    m_fd = line_request.fd;
    m_event.set_handle(m_fd);

    return true;
}

c_gpio::~c_gpio()
{
	m_event.stop();

    // close pin handle if open
    if (m_fd != -1)
        close(m_fd);

    m_fd = -1;
}

static mutex mtx; // lock mutex

bool c_gpio::read(uint32_t& val)
{
	c_guard guard(&mtx);

    gpio_v2_line_values line_values;
    line_values.mask = 1;
    line_values.bits = 0;

    // read from port
    if (ioctl(m_fd, GPIO_V2_LINE_GET_VALUES_IOCTL, &line_values))
        return set_error(ERR_SYS);

    val = (line_values.bits == 1) ? 1 : 0;

    return true;
}

// watch gpio for chenges
bool c_gpio::watch(s_gpio_event& gpio_event)
{
    // read event data
    gpio_v2_line_event event_data;

    while(1)
    {
    	// check if data to read
    	if (!m_event.poll_handle())
    		return false;

    	// read event data
		int32_t ret = ::read(m_fd, &event_data, sizeof(event_data));

		// check retun code
		if (ret == -1)
		{
			// read again
			if (errno == -EAGAIN)
				continue;
			else
				return false;
		}

		// check if read all data
		if (ret != sizeof(event_data))
			return false;

		break;
    }

    gpio_event.pin = m_pin;

    // set event edge
    switch(event_data.id)
    {
    case GPIO_V2_LINE_EVENT_RISING_EDGE:
    	gpio_event.edge = GPIO_EDGE_RISING;
        break;
    case GPIO_V2_LINE_EVENT_FALLING_EDGE:
    	gpio_event.edge = GPIO_EDGE_FALLING;
        break;
    default:
        return set_error(ERR_SYS);
    }

    // read input state
	if (!read(gpio_event.state))
        return set_error(ERR_SYS);

    return true;
}
