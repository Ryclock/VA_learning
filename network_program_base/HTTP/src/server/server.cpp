#include "server/server.h"
#include "utils/base64.h"
#include "utils/common.h"
#include "utils/log.h"

#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/http_struct.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs/legacy/constants_c.h>
#include <json/json.h>
#include <json/value.h>

const size_t recv_buf_max_size = 1024 * 1024 * 1;
char tmp_buf[recv_buf_max_size];

HTTP_Server::HTTP_Server()
{
    WSADATA wd_sock_msg;
    int s = WSAStartup(MAKEWORD(2, 2), &wd_sock_msg);
    if (0 != s)
    {
        switch (s)
        {
        case WSASYSNOTREADY:
            printf("Restart your computer, or check the network library");
            break;
        case WSAVERNOTSUPPORTED:
            printf("Please update the network library");
            break;
        case WSAEINPROGRESS:
            printf("Please restart");
            break;
        case WSAEPROCLIM:
            printf("Please turn off unnecessary software to ensure that there is enough network resources");
            break;
        default:
            break;
        }
    }

    if (2 != HIBYTE(wd_sock_msg.wVersion) || 2 != LOBYTE(wd_sock_msg.wVersion))
    {
        log_error("Network library version is wrong");
        return;
    }
}

HTTP_Server::~HTTP_Server()
{
    log_error("");
    WSACleanup();
}

void HTTP_Server::start(const char *ip, int port)
{
    event_config *config = event_config_new();
    struct event_base *base = event_base_new_with_config(config);
    struct evhttp *http = evhttp_new(base);

    evhttp_set_default_content_type(http, "text/html; charaset=utf-8");
    evhttp_set_timeout(http, 30);
    evhttp_set_cb(http, "/", index, this);
    evhttp_set_cb(http, "/api/health", api_health, this);
    evhttp_set_cb(http, "/api/data", api_data, this);
    evhttp_set_cb(http, "/api/upload_image", api_upload_image, this);
    evhttp_bind_socket(http, ip, port);
    event_base_dispatch(base);

    evhttp_free(http);
    event_base_free(base);
    event_config_free(config);
}

void index(struct evhttp_request *req, void *arg)
{
    log_info("");
    Json::Value res_urls, res;
    res_urls["/api"] = "api";
    res_urls["/api/health"] = "check health";
    res_urls["/api/data"] = "data";
    res_urls["/api/upload_image"] = "upload image";
    res["urls"] = res_urls;

    struct evbuffer *buf = evbuffer_new();
    evbuffer_add_printf(buf, "%s", res.toStyledString().c_str());
    evhttp_send_reply(req, HTTP_OK, nullptr, buf);
    evbuffer_free(buf);
}

void api_health(struct evhttp_request *req, void *arg)
{
    int res_code = 0;
    std::string res_msg = "error";

    res_code = 1000;
    res_msg = "current service health";

    Json::Value res;
    res["msg"] = res_msg;
    res["code"] = res_code;

    struct evbuffer *buff = evbuffer_new();
    evbuffer_add_printf(buff, "%s", res.toStyledString().c_str());
    evhttp_send_reply(req, HTTP_OK, nullptr, buff);
    evbuffer_free(buff);
}

void api_data(struct evhttp_request *req, void *arg)
{
    log_info("");
    HTTP_Server *server = (HTTP_Server *)arg;
    parse_post(req, tmp_buf);

    Json::CharReaderBuilder builder;
    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    Json::Value root;
    JSONCPP_STRING errors;

    Json::Value res, res_data, res_data_item;
    int res_code = 0;
    std::string res_msg = "error";
    if (reader->parse(tmp_buf, tmp_buf + std::strlen(tmp_buf), &root, &errors) && errors.empty())
    {
        for (int i = 0; i < 100; i++)
        {
            res_data_item["name"] = "name-" + std::to_string(i);
            res_data_item["seq"] = i;
            res_data_item["current timestamp"] = get_current_timestamp();
            res_data.append(res_data_item);
        }
        res["data"] = res_data;
        res_code = 1000;
        res_msg = "success";
    }
    else
    {
        res_msg = "invalid request parameter";
    }
    res["msg"] = res_msg;
    res["code"] = res_code;

    log_info("\n\trequest:%s\n\tresponse:%s", root.toStyledString().data(), res.toStyledString().data());

    struct evbuffer *buf = evbuffer_new();
    evbuffer_add_printf(buf, "%s", res.toStyledString().c_str());
    evhttp_send_reply(req, HTTP_OK, nullptr, buf);
    evbuffer_free(buf);
}

void api_upload_image(struct evhttp_request *req, void *arg)
{
    log_info("");
    parse_post(req, tmp_buf);

    Json::CharReaderBuilder builder;
    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    Json::Value root;
    JSONCPP_STRING errors;

    Json::Value res;
    int res_code = 0;
    std::string res_msg = "error";
    if (reader->parse(tmp_buf, tmp_buf + std::strlen(tmp_buf), &root, &errors) && errors.empty())
    {
        if (root["code"].isString())
        {
            std::string code = root["code"].asCString();
            log_info("code=%s", code.data());
        }
        if (root["image"].isString())
        {
            std::string file_name = "image-" + std::to_string(get_current_time()) + ".jpg";
            log_info("image=%s", file_name.data());

            std::string image_base64 = root["image"].asString();
            std::string image_str;
            base64::decode(image_base64, image_str);
            std::vector<char> image_buf(image_str.begin(), image_str.end());
            cv::Mat image = cv::imdecode(image_buf, CV_LOAD_IMAGE_COLOR);
            cv::imwrite(file_name, image);
        }
        res_code = 1000;
        res_msg = "success";
    }
    else
    {
        res_msg = "invalid request parameter";
    }
    res["msg"] = res_msg;
    res["code"] = res_code;

    struct evbuffer *buf = evbuffer_new();
    evbuffer_add_printf(buf, "%s", res.toStyledString().c_str());
    evhttp_send_reply(req, HTTP_OK, nullptr, buf);
    evbuffer_free(buf);
}

void parse_get(struct evhttp_request *req, struct evkeyvalq *params)
{
    if (req == nullptr)
    {
        return;
    }

    const char *url = evhttp_request_get_uri(req);
    if (url == nullptr)
    {
        return;
    }

    struct evhttp_uri *decoded = evhttp_uri_parse(url);
    if (!decoded)
    {
        return;
    }

    const char *path = evhttp_uri_get_path(decoded);
    if (path == nullptr)
    {
        path = "/";
    }

    char *query = (char *)evhttp_uri_get_query(decoded);
    if (query == nullptr)
    {
        return;
    }

    evhttp_parse_query_str(query, params);
}

void parse_post(struct evhttp_request *req, char *buf)
{
    size_t post_size = evbuffer_get_length(req->input_buffer);
    if (post_size <= 0)
    {
        log_error("====line:%d, post msg is empty", __LINE__);
        return;
    }

    size_t copy_len = std::min(post_size, recv_buf_max_size);
    log_info("post len:%d, copy_len:%d", post_size, copy_len);
    memcpy(buf, evbuffer_pullup(req->input_buffer, -1), copy_len);
    buf[post_size] = '\0';
    log_info("post msg:%s", buf);
}