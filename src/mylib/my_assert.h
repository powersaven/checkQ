/*
 * File:     my_assert.h
 * Date:     Dec 6, 2010
 * Author:   Haowei
 */
#ifndef MY_ASSERT_H
#define MY_ASSERT_H

#include <stdlib.h>

#include "log.h"

#ifdef HAVE_DEBUG

#define my_assert(statement) \
    do {                                                    \
        if (!(statement)) {                                 \
            log_error("Assertion failed: " #statement);     \
            abort();                                        \
        }                                                   \
    } while (0)

#else // HAVE_DEBUG

#define my_assert(statement)

#endif // HAVE_DEBUG

#endif // MY_ASSERT_H
