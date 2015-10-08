#include "log.h"

int
main(int argc, char *argv[]) {

    log_debug("This is a debug log");
    log_warn("This is a warn log");
    log_info("This is an info log");
    log_error("This is an error log");

}
