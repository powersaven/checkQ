#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "file_operation.h"
#include "mylib.h"

ret_t
file_is_exists(const char *file_path) {
    
    if ( 0 == access(file_path, F_OK) ) {
        return RET_SUCCEED;
    } else {
        return RET_FAILED;
    }
}

size_t
file_get_size(const char *file_path) {
    int my_fd;

    my_fd = open(file_path, O_RDONLY);
    if (-1 == my_fd) {
        return 0;
    } else {
        off_t my_off;
        my_off = lseek(my_fd, 0, SEEK_END);
        if (-1 == my_off) {
            return 0;
        }
        else {
            return my_off;
        }
    }
}

