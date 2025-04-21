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
#include "gpio_def.h"
#include "c_gpio.h"
#include "c_guard.h"
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

c_gpio::c_gpio()
{
    m_fd = -1;
    m_output = false;
}

c_gpio::~c_gpio()
{
    // close pin handle if open
    if (m_fd != -1)
        close(m_fd);
}

// set debounce
static void set_line_debounce_us(gpio_v2_line_config& line_config, uint32_t debounce)
{
    line_config.num_attrs = 1;
    line_config.attrs[0].mask = 1;
    line_config.attrs[0].attr.id = GPIO_V2_LINE_ATTR_ID_DEBOUNCE;
    line_config.attrs[0].attr.debounce_period_us = debounce;
}

// set output state
static void set_line_value(gpio_v2_line_config& line_config, uint32_t setval)
{
    line_config.num_attrs = 1;
    line_config.attrs[0].mask = 1;
    line_config.attrs[0].attr.id = GPIO_V2_LINE_ATTR_ID_OUTPUT_VALUES;
    line_config.attrs[0].attr.values = setval > 0 ? 1 : 0;
}

// init gpio
bool c_gpio::init(uint32_t pin, uint32_t mode, uint32_t setval)
{
    m_fd = -1;
    m_output = false;

    // check if chip is open
    if (chip.get_fd() == -1)
    {
        set_error(ERR_CHIP);
        return false;
    }

    // init line request
    gpio_v2_line_request line_request;

    memset(&line_request, 0, sizeof(line_request));

    line_request.num_lines = 1;
    line_request.offsets[0] = pin;
    line_request.config.flags = GPIO_V2_LINE_FLAG_OUTPUT;

    switch(mode)
    {
    case GPIO_MODE_INPUT_NOPULL:
        line_request.config.flags = GPIO_V2_LINE_FLAG_INPUT + GPIO_V2_LINE_FLAG_BIAS_DISABLED;
        set_line_debounce_us(line_request.config, setval);
        break;

    default:
    case GPIO_MODE_INPUT_PULLDOWN:
        line_request.config.flags = GPIO_V2_LINE_FLAG_INPUT + GPIO_V2_LINE_FLAG_BIAS_PULL_DOWN;
        set_line_debounce_us(line_request.config, setval);
        break;

    case GPIO_MODE_INPUT_PULLUP:
        line_request.config.flags = GPIO_V2_LINE_FLAG_INPUT + GPIO_V2_LINE_FLAG_BIAS_PULL_UP + GPIO_V2_LINE_FLAG_ACTIVE_LOW;
        set_line_debounce_us(line_request.config, setval);
        break;

    case GPIO_MODE_OUTPUT:
        line_request.config.flags = GPIO_V2_LINE_FLAG_OUTPUT;
        set_line_value(line_request.config, setval);
        m_output = true;
        break;

    case GPIO_MODE_OUTPUT_SOURCE:
        line_request.config.flags = GPIO_V2_LINE_FLAG_OUTPUT + GPIO_V2_LINE_FLAG_OPEN_SOURCE;
        setval = 0;
        set_line_value(line_request.config, setval);
        m_output = true;
        break;

    case GPIO_MODE_OUTPUT_SINK:
        line_request.config.flags = GPIO_V2_LINE_FLAG_OUTPUT + GPIO_V2_LINE_FLAG_OPEN_DRAIN + GPIO_V2_LINE_FLAG_ACTIVE_LOW;
        set_line_value(line_request.config, setval);
        m_output = true;
        break;
    }

    // set gpio pin
    if (ioctl(chip.get_fd(), GPIO_V2_GET_LINE_IOCTL, &line_request) == -1)
    {
        set_error(ERR_SYS);
        return false;
    }

    // check for valid handle
    if (line_request.fd < 0)
    {
        set_error(ERR_SYS);
        return false;
    }

    // set file handle
    m_fd = line_request.fd;

    return true;
}

// read gpio
bool c_gpio::read(uint32_t& val)
{
    c_guard guard(&m_mtx);

    gpio_v2_line_values line_values;
    line_values.mask = 1;
    line_values.bits = 0;

    if (ioctl(get_fd(), GPIO_V2_LINE_GET_VALUES_IOCTL, &line_values))
        return set_error(ERR_SYS);

    val = (line_values.bits == 1) ? 1 : 0;

    return true;
}

// write to gpio
bool c_gpio::write(uint32_t val)
{
    c_guard guard(&m_mtx);

    gpio_v2_line_values line_values;
    line_values.mask = 1;
    line_values.bits = val > 0 ? 1 : 0;

    // write gpio pin
    if (ioctl(get_fd(), GPIO_V2_LINE_SET_VALUES_IOCTL, &line_values) == -1)
        return set_error(ERR_SYS);

    return true;
}

// toggle gpio
bool c_gpio::toggle()
{
    c_guard guard(&m_mtx);

    gpio_v2_line_values line_values;
    line_values.mask = 1;
    line_values.bits = 0;

    if (ioctl(get_fd(), GPIO_V2_LINE_GET_VALUES_IOCTL, &line_values))
        return set_error(ERR_SYS);

    // toggle output
    line_values.bits = (line_values.bits > 0) ? 0 : 1;

    // write gpio pin
    if (ioctl(get_fd(), GPIO_V2_LINE_SET_VALUES_IOCTL, &line_values) == -1)
        return set_error(ERR_SYS);

    return true;
}

// **** c_gpioitem

c_gpioitem::c_gpioitem()
{
    m_gpio = NULL;
    m_blink = NULL;
    m_pulse = NULL;
    m_pwm = NULL;
}

c_gpioitem::~c_gpioitem()
{
    deinit();
}

void c_gpioitem::deinit()
{
    stopblink();
    stoppulse();
    stoppwm();

    if (m_gpio != NULL)
    {
        delete (c_gpio*) m_gpio;
        m_gpio = NULL;
    }
}

// init gpio item
bool c_gpioitem::init(uint32_t pin, uint32_t mode, uint32_t setval)
{
    if (m_gpio == NULL)
    {
        m_gpio = new c_gpio();

        if (!m_gpio->init(pin, mode, setval))
            return false;
    }
    else
        return set_error(ERR_ISINIT);

    return true;
}

// read gpio item
bool c_gpioitem::read(uint32_t& val)
{
    if (!isinit())
        return set_error(ERR_ISNOINIT);

    stopblink();
    stoppulse();
    stoppwm();

    return m_gpio->read(val);
}

// write gpio item
bool c_gpioitem::write(uint32_t val)
{
    if (!isinit())
        return set_error(ERR_ISNOINIT);

    if (!isoutput())
        return set_error(ERR_NOOUTPUT);

    stopblink();
    stoppulse();
    stoppwm();

    return m_gpio->write(val);
}

// toggle gpio item
bool c_gpioitem::toggle()
{
    if (!isinit())
        return set_error(ERR_ISNOINIT);

    if (!isoutput())
        return set_error(ERR_NOOUTPUT);

    stopblink();
    stoppulse();
    stoppwm();

    return m_gpio->toggle();
}

// blink gpio item
bool c_gpioitem::blink(uint32_t tblink)
{
    if (!isinit())
        return set_error(ERR_ISNOINIT);

    if (!isoutput())
        return set_error(ERR_NOOUTPUT);

    stoppulse();
    stoppwm();

    if (isblink())
    {
        if (tblink == 0)
            stopblink();
        else
            m_blink->set_tblink(tblink);
    }
    else
        m_blink = new c_blink(m_gpio, tblink);

    return true;
}

// pulse gpio item
bool c_gpioitem::pulse(uint32_t tpulse)
{
    if (!isinit())
        return set_error(ERR_ISNOINIT);

    if (!isoutput())
        return set_error(ERR_NOOUTPUT);

    stopblink();
    stoppulse();
    stoppwm();

    if (tpulse > 0)
        m_pulse = new c_pulse(m_gpio, tpulse);

    return true;
}

// pwm gpio item
bool c_gpioitem::pwm(uint32_t frequency, uint32_t dutycycle)
{
    if (!isinit())
        return set_error(ERR_ISNOINIT);

    if (!isoutput())
        return set_error(ERR_NOOUTPUT);

    stopblink();
    stoppulse();

    if (frequency == 0)
    {
        stoppwm();
        return true;
    }

    if (ispwm())
        m_pwm->set_data(frequency, dutycycle);
    else
        m_pwm = new c_pwm(m_gpio, frequency, dutycycle);

    return true;
}

#define DEF_FREQUENCY 400 // Hz
#define DEF_DUTYCYCLE 50 // %

// get gpio item pwm frequency
uint32_t c_gpioitem::get_frequency()
{
    if (ispwm())
        return m_pwm->get_frequency();

    return DEF_FREQUENCY;
}

// get gpio item pwm dutycycle
uint32_t c_gpioitem::get_dutycycle()
{
    if (ispwm())
        return m_pwm->get_dutycycle();

    return DEF_DUTYCYCLE;
}

// stop gpio item blink
void c_gpioitem::stopblink()
{
    if (m_blink != NULL)
    {
        delete (c_blink*) m_blink;
        m_blink = NULL;
    }
}

// stop gpio item pulse
void c_gpioitem::stoppulse()
{
    if (m_pulse != NULL)
    {
        delete (c_pulse*) m_pulse;
        m_pulse = NULL;
    }
}

// stop gpio item pwm
void c_gpioitem::stoppwm()
{
    if (m_pwm != NULL)
    {
        delete (c_pwm*) m_pwm;
        m_pwm = NULL;
    }
}
