#ifndef _BASE64_H
#define _BASE64_H

#include <string>

namespace base64
{
    inline bool is_base64(unsigned char c)
    {
        return (isalnum(c) || (c == '+') || (c == '/'));
    }

    void encode(unsigned char *, int, std::string &);
    void decode(std::string &, std::string &);
}

#endif // _BASE64_H