#include "../../include/output.hpp"



#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdarg>
#include <cstring>
#include <stdarg.h>
#include <ctime>


#define FFLUSH_BUFFER

void setColorStream(char fcolor, char bgcolor){
    static char buffer_edit_color[9] = "color --";
    buffer_edit_color[6] = fcolor;
    buffer_edit_color[7] = bgcolor;
    system(buffer_edit_color);
}
static FILE *file_log = NULL;
static time_t start = time(NULL);

static time_t start_all;

// clock_t start = clock();
//     double seconds = (double)(end - start) / CLOCKS_PER_SEC;
//     print_log(CONSOLE, "[%s:%d] %f clocks\n", __func__, __LINE__, seconds);

static std::string _log_filename;

int init_log(const char *log_filename){
    _log_filename = log_filename;
    file_log = fopen(_log_filename.c_str(), "w");
    if(!file_log){
        printf("[%s:%d] Error open: %s\n", __func__, __LINE__, _log_filename.c_str());
    }
    print_log(CONSOLE, "[%s:%d] \t\tINIT_LOG\n", __func__, __LINE__);
    // setColorStream('0', '9');
    start_all = time(NULL);
    return 0;
}
int deinit_log(){
   
    print_log(CONSOLE, "[DEINIT_LOG, time work program: %f]\n",  difftime(time(NULL), start_all));
    // setColorStream('0', '9');
    if(file_log){
        fclose(file_log);
        file_log = NULL;
    }
    return 0;
}
void print_log(int out, const char* format, ...){
    // return;
    va_list args;
    va_start(args, format);
    if(!file_log){
        printf("[%s:%d] Error", __func__, __LINE__);
        return;
    }

    switch (out){
    case CONSOLE:
        vfprintf(file_log, format, args);
        break;
    case LOG:
        vfprintf(file_log, format, args);
        break;
    case LOG_DATA:
        vfprintf(file_log, format, args);
        break;
    case DEBUG:
        vfprintf(file_log, format, args);
        break;
    case ERROR_OUT:
        vfprintf(file_log, format, args);
        break;
    case LOG_DEVICE:
        vfprintf(file_log, format, args);
        break;
    default:
        vfprintf(file_log, format, args);
        break;
    }
    va_end(args);
    va_start(args, format);
    switch (out){
    case CONSOLE:
        vfprintf(stdout, format, args);
        break;
    case LOG:
        vfprintf(stdout, format, args);
        break;
    case LOG_DATA:
        break;
    case DEBUG:
        // vfprintf(stdout, format, args);
        break;
    case ERROR_OUT:
        vfprintf(stdout, format, args);
        break;
    case LOG_DEVICE:
        vfprintf(stdout, format, args);
        break;
    default:
        break;
    }
    va_end(args);
#ifdef FFLUSH_BUFFER 
    fflush(file_log);
#endif
    
}

int write_file_bin_data(const char *filename, const void *data, const int size){
    FILE *file = fopen(filename, "wb");
    if(!file){
        print_log(CONSOLE, "[%s:%d] not found %s\n", __func__, __LINE__, filename);
        return -1;
    }
    fwrite((void*)&size, 1, sizeof(size), file);
    int res = fwrite(data, 1, size, file);
    // print_log(CONSOLE, "[%s:%d] write %d\n", __func__, __LINE__, res);
    fclose(file);
    return 0;
}

void time_counting_start(){
    start = time(NULL);
}
void time_counting_end(int out){
    double seconds_time = difftime(time(NULL), start);
    print_log(out, "[seconds = %f]\n", seconds_time);
}

void print_to_file(const char *filename, const char* key, const char* format, ...){
    FILE *file = fopen(filename, key);
    if(!file){
        print_log(CONSOLE, "Error: No open: %s, key: %s\n", filename, key);
        return;
    }
    if(format){
        va_list args;
        va_start(args, format);
        vfprintf(file, format, args);
        va_end(args);
    }
    fclose(file);
}

