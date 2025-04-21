/*
 * n-api parameter check
 *
 * (c) Derya Y. iiot2k@gmail.com
 *
 * exception.h
 *
 */

#pragma once

#define CHECK_PARAM_VOID(n_par)                                         \
for (uint32_t i_info = 0; i_info < n_par; i_info++)                     \
    if ((n_par != info.Length()) || info[i_info].IsUndefined())         \
    {                                                                   \
        Error::New(info.Env(), "invalid parameters").ThrowAsJavaScriptException();  \
        return;                                                         \
    }

#define CHECK_PARAM(n_par)                                                 \
for (uint32_t i_info = 0; i_info < n_par; i_info++)                     \
    if ((n_par != info.Length()) || info[i_info].IsUndefined())         \
    {                                                                   \
        Error::New(info.Env(), "invalid parameters").ThrowAsJavaScriptException();  \
        return info.Env().Undefined();                                  \
    }

