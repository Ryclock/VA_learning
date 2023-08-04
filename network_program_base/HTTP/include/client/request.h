#ifndef _REQUEST_H
#define _REQUEST_H

#include <string>

namespace request
{
    class Request
    {
    public:
        Request();
        ~Request();
        bool get(const char *, std::string &);
        bool post(const char *, const char *, std::string &);
    };
}
#endif // _REQUEST_H