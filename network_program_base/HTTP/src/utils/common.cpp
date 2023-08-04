#include <chrono>
#include <io.h>
#include "utils/common.h"

static int64_t get_current_time()
{
#ifndef WIN32
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &nom);
    return (now.tv_sec * 1000 + now.tc_nsec / 1000000);
#else
    long long now = std::chrono::steady_clock::now().time_since_epoch().count();
    return now / 1000000;
#endif
}

static int64_t get_current_timestamp()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
}

static std::vector<std::string> split(const std::string &str, const std::string &sep)
{
    std::vector<std::string> arr;
    int sep_size = sep.size();
    int last_pos = 0, index = -1;

    while (-1 != (index = str.find(sep, last_pos)))
    {
        arr.push_back(str.substr(last_pos, index - last_pos));
        last_pos = index + sep_size;
    }

    std::string last_str = str.substr(last_pos);
    if (!last_str.empty())
    {
        arr.push_back(last_str);
    }

    return arr;
}

static bool remove_file(const std::string &file_name)
{
    if (remove(file_name.data()) != 0)
    {
        return false;
    }
    return true;
}

static void mkdirs(const std::string &dir)
{
    if (::_access(dir.data(), 0) != 0)
    {
        std::string cmd;
        cmd = "mkdir -p" + dir;
        system(cmd.c_str());
    }
}
