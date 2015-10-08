#include <stdio.h>
#include <string.h>
#include <math.h>
#include "verify.h"

ret_t
verify_ip(const char *ip) {
    int fgm[4];

    if (4 != sscanf(ip, "%d.%d.%d.%d", &fgm[0], &fgm[1], &fgm[2], &fgm[3])) {
        return RET_FAILED;
    }
    for (int i = 0; i < 4; i ++) {
        if (fgm[i] < 0 || fgm[i] > 255) {
            return RET_FAILED;
        }
    }
    return RET_SUCCEED;
}

ret_t
verify_port(int port) {
    if (port >= 0 && port < 65536) {
        return RET_SUCCEED;
    } else {
        return RET_FAILED;
    }
}

ret_t
verify_string(const char *ori, const char *ref) {
    if (0 == strcmp(ori, ref)) {
        return RET_SUCCEED;
    } else {
        return RET_FAILED;
    }
}

ret_t
verify_rate(const double *rate) {
    double sum = 0.0;
    double eps = 0.01;

    for (int i = 0; i < 4; i ++) {
        if (rate[i] < 0 || rate[i] >= 1.0) {
            return RET_FAILED;
        } 
        sum += rate[i];
    }
    if (sum > 1.0 + eps) {
        return RET_FAILED;
    }

    return RET_SUCCEED;
}

ret_t
verify_window(int rows, int cols) {

    if (rows < 0 || cols < 0) {
        return RET_FAILED;
    }
    return RET_SUCCEED;
}

