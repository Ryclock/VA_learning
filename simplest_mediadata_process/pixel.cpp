#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include "pixel.h"

int split_yuv_420(const char *url, int w, int h, int n)
{
    // fstream is slower, especially in binary file
    std::FILE *ifp = std::fopen(url, "rb+");
    std::FILE *ofp_y = std::fopen("./output/420_y.y", "wb+");
    std::FILE *ofp_u = std::fopen("./output/420_u.y", "wb+");
    std::FILE *ofp_v = std::fopen("./output/420_v.y", "wb+");
    if (!ifp || !ofp_y || !ofp_u || !ofp_v)
    {
        std::printf("File Open Error");
        return -1;
    }

    // The vector cache is suitable for the object and the structure
    unsigned char *buf = (unsigned char *)std::malloc(w * h * 3 / 2);
    for (int i = 0; i < n; i++)
    {
        std::fread(buf, w * h * 3 / 2, 1, ifp);
        std::fwrite(buf, w * h, 1, ofp_y);
        std::fwrite(buf + w * h, w * h / 4, 1, ofp_u);
        std::fwrite(buf + w * h * 5 / 4, w * h / 4, 1, ofp_v);
    }

    std::free(buf);
    std::fclose(ifp);
    std::fclose(ofp_y);
    std::fclose(ofp_u);
    std::fclose(ofp_v);
    return 0;
}

int split_yuv_444(const char *url, int w, int h, int n)
{
    // fstream is slower, especially in binary file
    std::FILE *ifp = std::fopen(url, "rb+");
    std::FILE *ofp_y = std::fopen("./output/444_y.y", "wb+");
    std::FILE *ofp_u = std::fopen("./output/444_u.y", "wb+");
    std::FILE *ofp_v = std::fopen("./output/444_v.y", "wb+");
    if (!ifp || !ofp_y || !ofp_u || !ofp_v)
    {
        std::printf("File Open Error");
        return -1;
    }

    // The vector cache is suitable for the object and the structure
    unsigned char *buf = (unsigned char *)std::malloc(w * h * 3);
    for (int i = 0; i < n; i++)
    {
        std::fread(buf, w * h * 3, 1, ifp);
        std::fwrite(buf, w * h, 1, ofp_y);
        std::fwrite(buf + w * h, w * h, 1, ofp_u);
        std::fwrite(buf + w * h * 2, w * h, 1, ofp_v);
    }

    std::free(buf);
    std::fclose(ifp);
    std::fclose(ofp_y);
    std::fclose(ofp_u);
    std::fclose(ofp_v);
    return 0;
}

int split_rgb(const char *url, int w, int h, int n)
{
    // fstream is slower, especially in binary file
    std::FILE *ifp = std::fopen(url, "rb+");
    std::FILE *ofp_r = std::fopen("./output/r.y", "wb+");
    std::FILE *ofp_g = std::fopen("./output/g.y", "wb+");
    std::FILE *ofp_b = std::fopen("./output/b.y", "wb+");
    if (!ifp || !ofp_r || !ofp_g || !ofp_b)
    {
        std::printf("File Open Error");
        return -1;
    }

    // The vector cache is suitable for the object and the structure
    unsigned char *buf = (unsigned char *)std::malloc(w * h * 3);
    for (int i = 0; i < n; i++)
    {
        std::fread(buf, w * h * 3, 1, ifp);
        for (int j = 0; j < w * h * 3; j = j + 3)
        {
            std::fwrite(buf + j, 1, 1, ofp_r);
            std::fwrite(buf + j + 1, 1, 1, ofp_g);
            std::fwrite(buf + j + 2, 1, 1, ofp_b);
        }
    }

    std::free(buf);
    std::fclose(ifp);
    std::fclose(ofp_r);
    std::fclose(ofp_g);
    std::fclose(ofp_b);
    return 0;
}

int trans_yuv420_gray(const char *url, int w, int h, int n)
{
    std::FILE *ifp = std::fopen(url, "rb+");
    std::FILE *ofp = std::fopen("./output/gray420.yuv", "wb+");
    if (!ifp || !ofp)
    {
        std::printf("File Open Error");
        return -1;
    }

    unsigned char *buf = (unsigned char *)std::malloc(w * h * 3 / 2);
    for (int i = 0; i < n; i++)
    {
        std::fread(buf, w * h * 3 / 2, 1, ifp);
        std::memset(buf + w * h, 128, w * h / 2);
        std::fwrite(buf, w * h * 3 / 2, 1, ofp);
    }

    std::free(buf);
    std::fclose(ifp);
    std::fclose(ofp);
    return 0;
}

int trans_yuv420_halfy(const char *url, int w, int h, int n)
{
    std::FILE *ifp = std::fopen(url, "rb+");
    std::FILE *ofp = std::fopen("./output/halfy420.yuv", "wb+");
    if (!ifp || !ofp)
    {
        std::printf("File Open Error");
        return -1;
    }

    unsigned char *buf = (unsigned char *)std::malloc(w * h * 3 / 2);
    for (int i = 0; i < n; i++)
    {
        std::fread(buf, w * h * 3 / 2, 1, ifp);
        for (int j = 0; j < w * h; j++)
        {
            buf[j] = buf[j] >> 1;
        }
        std::fwrite(buf, w * h * 3 / 2, 1, ofp);
    }

    std::free(buf);
    std::fclose(ifp);
    std::fclose(ofp);
    return 0;
}

int trans_yuv420_border(const char *url, int w, int h, int n, int border)
{
    std::FILE *ifp = std::fopen(url, "rb+");
    std::FILE *ofp = std::fopen("./output/border420.yuv", "wb+");
    if (!ifp || !ofp)
    {
        std::printf("File Open Error");
        return -1;
    }

    unsigned char *buf = (unsigned char *)std::malloc(w * h * 3 / 2);
    for (int i = 0; i < n; i++)
    {
        std::fread(buf, w * h * 3 / 2, 1, ifp);
        for (int j = 0; j < h; j++)
        {
            for (int k = 0; k < w; k++)
            {
                if (k < border || j < border || k > (w - border) || j > (h - border))
                {
                    buf[j * w + k] = 0;
                }
            }
        }
        std::fwrite(buf, w * h * 3 / 2, 1, ofp);
    }

    std::free(buf);
    std::fclose(ifp);
    std::fclose(ofp);
    return 0;
}

int trans_rgb24_to_bmp(const char *url_in, const char *url_out, int w, int h)
{
    std::FILE *ifp = std::fopen(url_in, "rb+");
    std::FILE *ofp = std::fopen(url_out, "wb+");
    if (!ifp || !ofp)
    {
        std::printf("File Open Error");
        return -1;
    }

    // because of the memory alignment, variable 'bftype' need additional consideration
    char bftype[2] = {'B', 'M'};
    Bitmap_File_Header_Without_bftype bfh = {0};
    BitMap_Info_Header bih = {0};
    int header_size = sizeof(bftype) + sizeof(Bitmap_File_Header_Without_bftype) + sizeof(BitMap_Info_Header);
    bfh.bf_size = header_size + w * h * 3;
    bfh.bf_offset = header_size;
    bih.bi_size = sizeof(BitMap_Info_Header);
    bih.bi_width = w;
    // BMP storage pixel data in opposite direction of Y-axis (from bottom to top)
    bih.bi_height = -h;
    bih.bi_planes = 1;
    bih.bi_bitcount = 24;
    bih.bi_image_size = w * h * 3;

    unsigned char *buf = (unsigned char *)std::malloc(w * h * 3);
    std::fread(buf, w * h * 3, 1, ifp);
    // BMP save R1|G1|B1,R2|G2|B2 as B1|G1|R1,B2|G2|R2
    for (int i = 0; i < h; i++)
    {
        for (int j = 0; j < w; j++)
        {
            // '* 3' is important
            std::swap(buf[(i * w + j) * 3], buf[(i * w + j) * 3 + 2]);
        }
    }
    std::fwrite(bftype, sizeof(bftype), 1, ofp);
    std::fwrite(&bfh, sizeof(bfh), 1, ofp);
    std::fwrite(&bih, sizeof(bih), 1, ofp);
    std::fwrite(buf, w * h * 3, 1, ofp);

    std::free(buf);
    std::fclose(ifp);
    std::fclose(ofp);
    return 0;
}

int trans_rgb24_to_yuv420(const char *url_in, const char *url_out, int w, int h, int n)
{
    std::FILE *ifp = std::fopen(url_in, "rb+");
    std::FILE *ofp = std::fopen(url_out, "wb+");
    if (!ifp || !ofp)
    {
        std::printf("File Open Error");
        return -1;
    }

    unsigned char *buf_in = (unsigned char *)std::malloc(w * h * 3);
    unsigned char *buf_out = (unsigned char *)std::malloc(w * h * 3 / 2);
    for (int i = 0; i < n; i++)
    {
        std::fread(buf_in, w * h * 3, 1, ifp);
        std::memset(buf_out, 0, w * h * 3 / 2);
        unsigned char *ptr_y, *ptr_u, *ptr_v;
        ptr_y = buf_out;
        ptr_u = buf_out + w * h;
        ptr_v = buf_out + w * h * 5 / 4;
        for (int j = 0; j < h; j++)
        {
            // "* 3" is important
            unsigned char *ptr_rgb = buf_in + j * w * 3;
            for (int k = 0; k < w; k++)
            {
                unsigned char r, g, b, y;
                r = *(ptr_rgb++);
                g = *(ptr_rgb++);
                b = *(ptr_rgb++);
                // use digital y|u|v here, while analog y|u|v is different
                // y = 0.229 * r + 0.587 * g + 0.114 * b
                // u = -0.147 * r - 0.289 * g + 0.463 * b
                // v = 0.615 * r - 0.515 * g - 0.100 * b
                y = (unsigned char)((66 * r + 129 * g + 25 * b + 128) >> 8) + 16;
                *(ptr_y++) = y > 255 ? 255 : y < 0 ? 0
                                                   : y;
                if (k % 2 != 0)
                {
                    continue;
                }
                if (j % 2 != 0)
                {
                    unsigned char v = (unsigned char)((112 * r - 94 * g - 18 * b + 128) >> 8) + 128;
                    *(ptr_v++) = v > 255 ? 255 : v < 0 ? 0
                                                       : v;
                }
                else
                {
                    unsigned char u = (unsigned char)((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128;
                    *(ptr_u++) = u > 255 ? 255 : u < 0 ? 0
                                                       : u;
                }
            }
        }
        std::fwrite(buf_out, w * h * 3 / 2, 1, ofp);
    }

    std::free(buf_in);
    std::free(buf_out);
    std::fclose(ifp);
    std::fclose(ofp);
    return 0;
}

int generate_yuv420_graybar(const char *url, int w, int h, int y_min, int y_max, int bar_n)
{
    std::FILE *ofp = std::fopen(url, "wb+");
    if (!ofp)
    {
        std::printf("File Open Error");
        return -1;
    }

    unsigned char *buf = (unsigned char *)std::malloc(w * h * 3 / 2);
    int bar_width = w / bar_n;
    float delta_y = (float)(y_max - y_min) / (float)(bar_n - 1);
    for (int i = 0; i < h; i++)
    {
        for (int j = 0; j < w; j++)
        {
            int rank = j / bar_width;
            buf[i * w + j] = y_min + (char)(rank * delta_y);
        }
    }
    std::memset(buf + w * h, 128, w * h / 4);
    std::memset(buf + w * h * 5 / 4, 128, w * h / 4);
    std::fwrite(buf, w * h * 3 / 2, 1, ofp);

    std::free(buf);
    std::fclose(ofp);
    return 0;
}

int evaluate_yuv420_PSNRy(const char *url1, const char *url2, int w, int h, int n)
{
    std::FILE *ifp1 = std::fopen(url1, "rb+");
    std::FILE *ifp2 = std::fopen(url2, "rb+");
    if (!ifp1 || !ifp2)
    {
        std::printf("File Open Error");
        return -1;
    }

    unsigned char *buf1 = (unsigned char *)std::malloc(w * h);
    unsigned char *buf2 = (unsigned char *)std::malloc(w * h);
    for (int i = 0; i < n; i++)
    {
        std::fread(buf1, w * h, 1, ifp1);
        std::fread(buf2, w * h, 1, ifp2);
        double mse = 0, psnr = 0;
        for (int j = 0; j < w * h; j++)
        {
            mse += (double)((buf1[j] - buf2[j]) * (buf1[j] - buf2[j]));
        }
        mse /= (w * h);
        psnr = 10 * std::log10(255.0 * 255.0 / mse);
        std::printf("%5.3f\n", psnr);
    }

    return 0;
}
