/*
 * error code handling
 *
 * (c) Derya Y. iiot2k@gmail.com
 *
 * error_code.cpp
 *
 */

#include <atomic>
#include <string.h>
using namespace std;

#include "errno.h"

#include "error_code.h"

static atomic_uint32_t global_error_code;

const char* get_error_text()
{
    switch(global_error_code)
    {
    case ERR_NO: return "ok";

    case ERR_PARAMETER: return "inv. parameter";
    case ERR_PIN:       return "inv. pin";
    case ERR_ISNOINIT:  return "pin not init";
    case ERR_ISINIT:    return "pin is in use";

    case ERR_SYS: return strerror(errno);
    case ERR_CHIP: return "chip error";
    }

    return "unknown error";
}

const char* get_sys_error_text()
{
    return strerror(errno);
}

void clear_error()
{
    errno = 0;
    global_error_code = ERR_NO;
}

bool set_error(uint32_t code)
{
    if (global_error_code != ERR_NO)
        return false;

    global_error_code = code;
    return false;
}
