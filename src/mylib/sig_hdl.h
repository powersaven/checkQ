#ifndef SIG_HDL_H
#define SIG_HDL_H


void init_signal_handlers() __attribute__((constructor));

typedef void (*sig_quit_handler_t)(void *arg);

// Set a quit function for quitting signals like
// SIGINT, SIGTERM, SIGQUIT
void sig_set_quit_func(sig_quit_handler_t handler, void *arg);

typedef void (*signal_init_func_t)();

static signal_init_func_t signal_init_func __attribute__((unused))
    = init_signal_handlers;

#endif // SIG_HDL_H
