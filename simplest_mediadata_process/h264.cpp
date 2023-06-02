#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "h264.h"

int parse_h264(const char *url)
{
    std::FILE *ifp = std::fopen(url, "rb+");
    std::FILE *ofp = std::fopen("./output/h264.txt", "wb+");
    // std::FILE *ofp = stdout;
    if (!ifp || !ofp)
    {
        std::printf("File Open Error");
        return -1;
    }

    NALU_Header nh;
    char *buf = (char *)std::malloc(1024 * 1024);
    if (buf == NULL)
    {
        std::printf("Buf memory bad");
        return -1;
    }
    int nalu_n = 0;
    int data_offset = 0;
    std::fprintf(ofp, "-----+-------- NALU Table ------+---------+\n");
    std::fprintf(ofp, " NUM |   POS   |  IDC  |  TYPE  |   LEN   |\n");
    std::fprintf(ofp, "-----+---------+-------+--------+---------+\n");
    while (!std::feof(ifp))
    {
        int pos = 0;
        int startcode_len, data_len, next_startcode_len;
        // fread(des,size,num,src) where num determines its return value.
        if (3 != std::fread(buf, 1, 3, ifp))
        {
            break;
        }
        if (buf[0] != 0 || buf[1] != 0)
        {
            std::free(buf);
            return -1;
        }
        if (buf[2] == 1)
        {
            pos = startcode_len = 3;
        }
        else
        {
            if (1 != fread(buf + 3, 1, 1, ifp))
            {
                break;
            }
            if (buf[3] != 1)
            {
                std::free(buf);
                return -1;
            }
            pos = startcode_len = 4;
        }

        while (true)
        {
            if (feof(ifp))
            {
                data_len = pos - 1 - startcode_len;
                // memcpy is a good idea when structure with bit field is hard to assign.
                std::memcpy(&nh, buf + startcode_len, 1);
            }
            // segmentation fault here is likely to choose a small array.
            buf[pos++] = fgetc(ifp);

            // judging next startcode is all to step forward from "pos-1".
            if (buf[pos - 1] != 1 || buf[pos - 2] != 0 || buf[pos - 3] != 0)
            {
                continue;
            }
            next_startcode_len = (buf[pos - 4] == 0) ? 4 : 3;
            break;
        }

        // note that move in the right direction
        std::fseek(ifp, -next_startcode_len, SEEK_CUR);
        data_len = pos - next_startcode_len - startcode_len;
        std::memcpy(&nh, buf + startcode_len, 1);

        std::fprintf(ofp, "%5d| %8d| %6d| %7d| %8d|\n", nalu_n, data_offset, nh.NRI, nh.type, data_len);
        data_offset += data_len + startcode_len;
        nalu_n++;
    }
    std::fprintf(ofp, "-----+----------- End. ---------+---------+\n");

    std::free(buf);
    std::fclose(ifp);
    std::fclose(ofp);
    return 0;
}