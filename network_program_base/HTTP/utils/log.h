#ifndef _LOG_H
#define _LOG_H

#include <string>

static std::string log_time();
void log_info(const char *format, ...);
void log_error(const char *format, ...);

#endif // _LOG_H