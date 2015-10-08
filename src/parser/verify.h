#ifndef VERIFY_H
#define VERIFY_H

#include "return.h"

ret_t
verify_ip(const char *ip);

ret_t
verify_port(int port);

ret_t
verify_string(const char *ori, const char *ref);

ret_t
verify_rate(const double *rate);

ret_t
verify_window(int rows, int cols);

#endif
