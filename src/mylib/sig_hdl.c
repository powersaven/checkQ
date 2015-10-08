#include <signal.h>

#include "mylib.h"
#include "log.h"
#include "return.h"
#include "my_mutex.h"
#include "sig_hdl.h"

#define signal_table \
    /*   name    sig        handle */      \
    item(int,    SIGINT,    sig_hdl_int)   \
    item(segv,   SIGSEGV,   sig_hdl_segv)  \
    item(pipe,   SIGPIPE,   SIG_IGN)       \
    item(chld,   SIGCHLD,   SIG_IGN)       \
    item(abrt,   SIGABRT,   sig_hdl_abrt)  \
    item(term,   SIGTERM,   sig_hdl_term)  \
    item(quit,   SIGQUIT,   sig_hdl_quit)  \

static void sig_hdl_int(int signo attr_unused);
static void sig_hdl_segv(int signo attr_unused);
static void sig_hdl_abrt(int signo attr_unused);
static void sig_hdl_term(int signo attr_unused);
static void sig_hdl_quit(int signo attr_unused);

#define item(name, sig, handle) \
    static my_lock_t sig_lock_ ## name attr_unused = MY_LOCK_INITER;
signal_table
#undef item

static sig_quit_handler_t quit_handler = NULL;
static void *quit_handler_arg = NULL;

void 
sig_set_quit_func(sig_quit_handler_t handler, void *arg) {
    quit_handler = handler;
    quit_handler_arg = arg;
}

void 
init_signal_handlers() {
    #define item(name, sig, handle) signal(sig, handle);
    signal_table
    #undef item
}

static void
sig_hdl_int(int signo) {
    my_lock(&sig_lock_int);

    log_warn("SIGINIT handled.");

    if (quit_handler != NULL)
        (*quit_handler)(quit_handler_arg);

    exit(RET_SUCCEED);

    my_unlock(&sig_lock_int);
}

static void
sig_hdl_segv(int signo) {
    my_lock(&sig_lock_segv);

    log_error("Program received SIGSEGV.");

    exit(RET_FAILED);

    my_unlock(&sig_lock_segv);
}

static void
sig_hdl_abrt(int signo) {
    my_lock(&sig_lock_abrt);

    log_error("Program received SIGABRT.");

    exit(RET_FAILED);

    my_unlock(&sig_lock_abrt);
}

static void
sig_hdl_term(int signo) {
    my_lock(&sig_lock_term);

    log_warn("SIGTERM handled.");

    if (quit_handler != NULL)
        (*quit_handler)(quit_handler_arg);

    exit(RET_SUCCEED);

    my_unlock(&sig_lock_term);
}

static void
sig_hdl_quit(int signo) {
    my_lock(&sig_lock_quit);
    
    log_error("SIGQUIT handled.");

    if (quit_handler != NULL)
        (*quit_handler)(quit_handler_arg);

    exit(RET_FAILED);

    my_unlock(&sig_lock_quit);
}
