#ifndef _LOG_H
#define _LOG_H
#include <time.h>
#include <string>

static std::string log_time()
{
    const char *time_fmt = "%Y-%m-%d %H:%M:%S";
    time_t t = time(nullptr);
    char time_str[64];
    strftime(time_str, sizeof(time_str), time_fmt, localtime(&t));
    return time_str;
}

#define LOG_INFO(format, ...) fprintf(stdout, "\n[INFO]%s [%s:%d] " format, log_time().data(), __func__, __LINE__, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) fprintf(stderr, "\n[ERROR]%s [%s:%d] " format, log_time().data(), __func__, __LINE__, ##__VA_ARGS__)

#endif // _LOG_H