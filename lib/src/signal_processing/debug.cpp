#include "debug.hpp"
#include <stdio.h>
#include <stdarg.h>

static void (*log_output)(int, const char*, ...) = nullptr;


bool init_log_lib(void (*cb)(int, const char*, ...)) {
    log_output = cb;
}

inline void print_log_lib(int out, const char* format, ...) {
    if(!log_output) {
        return;
    }
    va_list args;
    va_start(args, format);
    log_output(out, format, args);
    va_end(args);
}




