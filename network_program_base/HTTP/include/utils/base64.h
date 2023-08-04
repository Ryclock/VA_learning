#ifndef _BASE64_H
#define _BASE64_H

#include <string>

namespace base64
{
    static inline bool is_base64(unsigned char c)
    {
        return (isalnum(c) || (c == '+') || (c == '/'));
    }

    static void encode(unsigned char *, int, std::string &);
    static void decode(std::string &, std::string &);
}

#endif // _BASE64_H