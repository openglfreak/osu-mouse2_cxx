#ifndef __COMMON_H__
#define __COMMON_H__

#if defined(UNICODE) && !defined(_UNICODE)
    #define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
    #define UNICODE
#endif

#include <tchar.h>
#include <stdio.h>

#if DEBUG
#define DBG_LINE(...) __VA_ARGS__
#else
#define DBG_LINE(...)
#endif // DEBUG
#define DBG_OUT(outp) DBG_LINE(_putts(outp))
#define DBG_OUTT(outp) DBG_OUT(_T(outp))

#include "resources.h"

#endif // __COMMON_H__
