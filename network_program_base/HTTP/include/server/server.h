#ifndef _HTTP_SERVER_H
#define _HTTP_SERVER_H

class HTTP_Server
{
public:
    explicit HTTP_Server();
    ~HTTP_Server();
    void start(const char *, int);
};

void index(struct evhttp_request *, void *);
void api_health(struct evhttp_request *, void *);
void api_data(struct evhttp_request *, void *);
void api_upload_image(struct evhttp_request *, void *);
void parse_get(struct evhttp_request *, struct evkeyvalq *);
void parse_post(struct evhttp_request *, char *);

#endif