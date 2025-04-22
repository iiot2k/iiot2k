/*
 * error code handling
 *
 * (c) Derya Y. iiot2k@gmail.com
 *
 * error_code.h
 *
 */

#pragma once

#include <stdint.h>

enum {
    ERR_NO = 0,
    ERR_PARAMETER,
    ERR_PIN,
    ERR_EDGE,
    ERR_MODE,
    ERR_RES,
    ERR_ISINIT,
    ERR_ISNOINIT,
    ERR_TIMEOUT,

    ERR_SYS = 1000,
    ERR_CHIP,
    ERR_NOSENSOR,
};

const char* get_error_text();
const char* get_sys_error_text();
void clear_error();
bool set_error(uint32_t code);
