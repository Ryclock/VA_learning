#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "aac.h"

int get_ADTS_frame(unsigned char *buf, int buf_size, ADTS_Header *ah, int *data_size)
{
    if (!buf || !ah || !data_size)
    {
        return -1;
    }

    int size = 0;
    while (true)
    {
        if (buf_size < 7)
        {
            return -1;
        }
        if ((buf[0] == 0xff) && (buf[1] & 0xf0 == 0xf0))
        {
            size |= (buf[3] & 0x03) << 11;
            size |= buf[4] << 3;
            size |= (buf[5] & 0xe0) >> 5;
            break;
        }
        buf_size--;
        buf++;
    }
    if (buf_size < size)
    {
        return 1;
    }

    std::memcpy(ah, buf, 7);
    *data_size = size;
    return 0;
}

int parse_acc(const char *url)
{
    std::FILE *ifp = std::fopen(url, "rb+");
    std::FILE *ofp = std::fopen("./output/aac.txt", "wb+");
    // std::FILE *ofp = stdout;
    if (!ifp || !ofp)
    {
        std::printf("File Open Error");
        return -1;
    }

    ADTS_Header ah;
    unsigned char *buf = (unsigned char *)std::malloc(1024 * 1024);
    if (buf == NULL)
    {
        std::printf("Buf memory bad");
        return -1;
    }
    int frame_n = 0;
    int pre_offset = 0;
    std::fprintf(ofp, "-----+- ADTS Frame Table -+------+\n");
    std::fprintf(ofp, " NUM | Profile | Frequency| Size |\n");
    std::fprintf(ofp, "-----+---------+----------+------+\n");
    while (!std::feof(ifp))
    {
        int buf_left_size = std::fread(buf + pre_offset, 1, 1024 * 1024 - pre_offset, ifp);
        unsigned char *buf_sub = buf;
        int data_size = 0;
        while (true)
        {
            int res = get_ADTS_frame(buf_sub, buf_left_size, &ah, &data_size);
            if (res == -1)
            {
                break;
            }
            else if (res == 1)
            {
                pre_offset = buf_left_size;
                std::memcpy(buf, buf_sub, pre_offset);
                break;
            }

            std::fprintf(ofp, "%5d| %8d| %9d| %5d|\n", frame_n, ah.profile, ah.sampling_frequency_index, data_size);
            buf_left_size -= data_size;
            buf_sub += data_size;
            frame_n++;
        }
    }
    std::fprintf(ofp, "-----+------- End. -------+------+\n");

    std::free(buf);
    std::fclose(ifp);
    std::fclose(ofp);
    return 0;
}