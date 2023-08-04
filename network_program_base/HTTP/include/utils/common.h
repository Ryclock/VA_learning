#ifndef _COMMON_H
#define _COMMON_H

#include <string>
#include <vector>

int64_t get_current_time();
int64_t get_current_timestamp();
std::vector<std::string> split(const std::string &str, const std::string &sep);
bool remove_file(const std::string &file_name);
void mkdirs(const std::string &dir);

#endif // _COMMON_H