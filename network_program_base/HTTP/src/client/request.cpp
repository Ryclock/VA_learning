#include "client/request.h"
#include <curl/curl.h>
#include "utils/log.h"

namespace request
{
    inline size_t on_write(void *buf, size_t size, size_t n_member, void *stream)
    {

        std::string *str = dynamic_cast<std::string *>((std::string *)stream);
        if (NULL == str || NULL == buf)
        {
            return -1;
        }

        char *p_data = (char *)buf;
        str->append(p_data, size * n_member);
        return n_member;
    }

    Request::Request()
    {
    }

    Request::~Request()
    {
    }

    bool Request::get(const char *url, std::string &response)
    {

        CURL *curl = curl_easy_init();
        if (!curl)
        {
            log_error("curl_easy_init error");
            return false;
        }

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);

        curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, on_write);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);

        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);

        CURLcode code = curl_easy_perform(curl);
        if (code != CURLE_OK)
        {
            log_error("curl_easy_strerror: %s", curl_easy_strerror(code));
            curl_easy_cleanup(curl);
            return false;
        }

        curl_easy_cleanup(curl);
        return true;
    }

    bool Request::post(const char *url, const char *data, std::string &response)
    {
        curl_global_init(CURL_GLOBAL_WIN32);

        CURL *curl = curl_easy_init();
        if (!curl)
        {
            log_error("curl_easy_init error: url=%s", url);
            curl_global_cleanup();
            return false;
        }

        struct curl_slist *headers = nullptr;
        headers = curl_slist_append(headers, "User-Agent: HTTP Client;");
        headers = curl_slist_append(headers, "Content-Type:application/json;");
        headers = curl_slist_append(headers, "expect: ;");

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_HEADER, 0);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);

        curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, on_write);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);

        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);

        CURLcode code = curl_easy_perform(curl);
        if (code != CURLE_OK)
        {
            log_error("curl_easy_strerror: url=%s, %s", url, curl_easy_strerror(code));
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return false;
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return true;
    }
}