#include <json/json.h>
#include <opencv2/opencv.hpp>
#include "client/request.h"
#include "utils/log.h"
#include "utils/base64.h"
#include "utils/common.h"

void test_upload_image()
{
    cv::Mat image = cv::imread("D:/tmp/test.jpg");
    std::string image_base64;
    std::vector<int> quality = {cv::IMWRITE_JPEG_QUALITY, 100};
    std::vector<uchar> jpeg_data;
    cv::imencode(".jpg", image, jpeg_data, quality);
    base64::encode(jpeg_data.data(), jpeg_data.size(), image_base64);

    Json::Value param;
    param["code"] = "s84dsd#7hf34r3jsk@fs$d#$dd";
    param["image"] = image_base64;
    std::string data = param.toStyledString();

    std::string url = "http://127.0.0.1:8080/api/upload_image";
    std::string response;
    request::Request request;
    int64_t t1 = get_current_time();
    bool res = request.post(url.data(), data.data(), response);
    int64_t t2 = get_current_time();
    log_info("request spend: %lld(ms), response=%s", (t2 - t1), response.data());
}

int main(int argc, char **argv)
{
    test_upload_image();
    return 0;
}