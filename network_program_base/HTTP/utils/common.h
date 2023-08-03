#ifndef _COMMON_H
#define _COMMON_H

#include <string>
#include <vector>

static int64_t get_current_time();
static int64_t get_current_timestamp();
static std::vector<std::string> split(const std::string &str, const std::string &sep);
static bool remove_file(const std::string &file_name);
static void mkdirs(const std::string &dir);

#endif // _COMMON_H