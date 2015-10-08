#ifndef SIM_MUTEX_H
#define SIM_MUTEX_H

#include <pthread.h>
#include "return.h"

typedef struct {
    pthread_mutex_t  mutex;
    pthread_cond_t   cond;
    int              flag;
} my_cond_t;

#define MY_COND_INITER \
    {PTHREAD_MUTEX_INITIALIZER,                                                \
     PTHREAD_COND_INITIALIZER, 0}

ret_t
my_cond_wait(my_cond_t *cond);

#define my_cond_wait_execute(my_cond, statement) \
    do {                                                             \
        pthread_mutex_lock(&(my_cond)->mutex);                       \
        while ((my_cond)->flag == 0)                                 \
            pthread_cond_wait(&(my_cond)->cond, &(my_cond)->mutex);  \
        (my_cond)->flag = 0;                                         \
        statement;                                                   \
        pthread_mutex_unlock(&(my_cond)->mutex);                     \
    } while (0)

ret_t
my_cond_timedwait(my_cond_t *cond, double seconds);

#define my_cond_timedwait_execute(my_cond, statement, second, status) \
    do {                                                             \
        pthread_mutex_lock(&(my_cond)->mutex);                       \
        while ((my_cond)->flag == 0)                                 \
            pthread_cond_wait(&(my_cond)->cond, &(my_cond)->mutex);  \
        (my_cond)->flag = 0;                                         \
        statement;                                                   \
        pthread_mutex_unlock(&(my_cond)->mutex);                     \
    } while (0)

ret_t
my_cond_signal(my_cond_t *cond);

#define my_cond_signal_execute(my_cond, statement) \
    do {                                                             \
	    pthread_mutex_lock(&(my_cond)->mutex);                       \
	    (my_cond)->flag = 1;                                         \
	    statement;                                                   \
	    pthread_cond_signal(&(my_cond)->cond);                       \
	    pthread_mutex_unlock(&(my_cond)->mutex);                     \
	} while (0)

typedef pthread_mutex_t my_lock_t;

#define MY_LOCK_INITER PTHREAD_MUTEX_INITIALIZER
#define my_lock pthread_mutex_lock
#define my_unlock pthread_mutex_unlock

typedef pthread_rwlock_t my_rwlock_t;

#define MY_RWLOCK_INITER PTHREAD_RWLOCK_INITIALIZER
#define my_rdlock pthread_rwlock_rdlock
#define my_wrlock pthread_rwlock_wrlock
#define my_rwunlock pthread_rwlock_unlock

#endif /* SIM_MUTEX_H */
