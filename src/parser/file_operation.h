#ifndef FILE_OPERATION_H
#define FILE_OPERATION_H

#include "return.h"

ret_t
file_is_exists(const char *file_path);

size_t
file_get_size(const char *file_path);

#endif
