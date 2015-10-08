#include <pthread.h>
#include <sys/time.h>

#include "my_mutex.h"

ret_t
my_cond_wait(my_cond_t *cond) {
    pthread_mutex_lock(&cond->mutex);
    while (cond->flag == 0)
        pthread_cond_wait(&cond->cond, &cond->mutex);
    cond->flag = 0;
    pthread_mutex_unlock(&cond->mutex);

    return RET_SUCCEED;
}

ret_t
my_cond_timedwait(my_cond_t *cond, double seconds) {
    ret_t ret = RET_SUCCEED;
    struct timeval now;
    struct timespec wait_time;

    gettimeofday(&now, NULL);

    time_t tv_sec = (int)seconds;
    time_t tv_nsec = (seconds - tv_sec) * 1000000000L;

    wait_time.tv_sec = now.tv_sec + tv_sec;
    wait_time.tv_nsec = now.tv_usec * 1000 + tv_nsec;
    if (wait_time.tv_nsec > 1000000000L) {
        wait_time.tv_nsec -= 1000000000L;
        wait_time.tv_sec ++;
    }

    pthread_mutex_lock(&cond->mutex);
    if (cond->flag == 0 &&
        pthread_cond_timedwait(&cond->cond, &cond->mutex, &wait_time) == 0) {
        cond->flag = 0;
    } else {
        ret = RET_FAILED;
    }
    pthread_mutex_unlock(&cond->mutex);

    return ret;
}

ret_t
my_cond_signal(my_cond_t *cond) {
    pthread_mutex_lock(&cond->mutex);
    cond->flag = 1;
    pthread_cond_signal(&cond->cond);
    pthread_mutex_unlock(&cond->mutex);

    return RET_SUCCEED;
}
