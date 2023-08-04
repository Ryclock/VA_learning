#include "utils/log.h"
#include <time.h>
#include <cstdarg>

std::string log_time()
{
    const char *time_fmt = "%Y-%m-%d %H:%M:%S";
    time_t t = time(nullptr);
    char time_str[64];
    strftime(time_str, sizeof(time_str), time_fmt, localtime(&t));
    return time_str;
}

void log_info(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    fprintf(stdout, "\n[INFO]%s [%s:%d] ", log_time().data(), __func__, __LINE__);
    vfprintf(stdout, format, args);
    va_end(args);
}

void log_error(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    fprintf(stderr, "\n[ERROR]%s [%s:%d] ", log_time().data(), __func__, __LINE__);
    vfprintf(stderr, format, args);
    va_end(args);
}