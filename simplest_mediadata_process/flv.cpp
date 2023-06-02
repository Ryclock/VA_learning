#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "flv.h"

int trans_endian_mode(const unsigned char *ptr, const int byte_size)
{
    unsigned int res = 0;
    for (int i = 0; i < byte_size; i++)
    {
        res |= (*(ptr + i) << (byte_size - 1 - i) * 8);
    }
    return res;
}

int parse_flv(const char *url)
{
    std::FILE *ifp = std::fopen(url, "rb+");
    std::FILE *ofp = std::fopen("./output/flv.txt", "wb+");
    // std::FILE *ofp = stdout;
    std::FILE *ofp_v = std::fopen("./output/flv.flv", "wb+");
    std::FILE *ofp_a = std::fopen("./output/flv.mp3", "wb+");
    if (!ifp || !ofp || !ofp_v || !ofp_a)
    {
        std::printf("File Open Error");
        return -1;
    }

    FLV_Header fh;
    Tag_Header th;
    Video_Tag_Header vth;
    Audio_Tag_Header ath;
    std::fread(&fh, sizeof(FLV_Header), 1, ifp);
    std::fprintf(ofp, "\n============== FLV Header ==============");
    std::fprintf(ofp, "\nSignature:  %c %c %c", fh.signature[0], fh.signature[1], fh.signature[2]);
    std::fprintf(ofp, "\nVersion:    0x%x", fh.version);
    std::fprintf(ofp, "\nFlags  :    0x%x", fh.flags);
    std::fprintf(ofp, "\nHeaderSize: 0x%x", trans_endian_mode(fh.header_size, sizeof(fh.header_size)));
    std::fprintf(ofp, "\n========================================");

    int pre_video_tag_size = 0, tag_data_size, cnt = 0;
    char pre_tag_size[4];
    while (!std::feof(ifp))
    {
        // all multi-byte is "Big Endian".
        std::fread(pre_tag_size, 4, 1, ifp);
        std::fread(&th, sizeof(Tag_Header), 1, ifp);
        tag_data_size = trans_endian_mode(th.data_size, sizeof(th.data_size));
        std::fprintf(ofp, "\n0x[%04x] %6d %6d |", th.type, tag_data_size,
                     trans_endian_mode(th.time_stamp, sizeof(th.time_stamp)));
        if (std::feof(ifp))
        {
            break;
        }

        if (th.type == 0x08)
        {
            std::fread(&ath, sizeof(Audio_Tag_Header), 1, ifp);
            std::fprintf(ofp, " 0x%x 0x%x 0x%x 0x%x", ath.encoding_type, ath.sampling_rate, ath.precision, ath.type);
            // fwrite can maybe not effective, from a file pointer to another file pointer.
            for (int i = 0; i < tag_data_size - sizeof(Audio_Tag_Header); i++)
            {
                std::fputc(std::fgetc(ifp), ofp_a);
            }
        }
        else if (th.type == 0x09)
        {
            std::fread(&vth, sizeof(Video_Tag_Header), 1, ifp);
            std::fprintf(ofp, " 0x%04x  0x%04x", vth.frame_type, vth.encoding_type);
            int pos = std::ftell(ofp_v);
            // ftell will return -1L when error happens.
            if (pos == -1L)
            {
                std::fprintf(ofp, "| ftell wrong");
                break;
            }
            else if (pos == 0)
            {
                std::fwrite(&fh, sizeof(FLV_Header), 1, ofp_v);
                std::fwrite(&pre_video_tag_size, sizeof(pre_video_tag_size), 1, ofp_v);
            }
            std::fwrite(&th, sizeof(Tag_Header), 1, ofp_v);
            std::fwrite(&vth, sizeof(Video_Tag_Header), 1, ofp_v);
            // fwrite can maybe not effective, from a file pointer to another file pointer.
            for (int i = 0; i < tag_data_size - sizeof(Video_Tag_Header) + 4; i++)
            {
                std::fputc(std::fgetc(ifp), ofp_v);
            }
            // pre_video_tag_size has been written in advance, in order to avoid change endian mode.
            std::fseek(ifp, -4, SEEK_CUR);
        }
        else
        {
            std::fseek(ifp, tag_data_size, SEEK_CUR);
        }
    }
    std::fprintf(ofp, "\n================= End. =================");

    std::fclose(ifp);
    std::fclose(ofp);
    std::fclose(ofp_v);
    std::fclose(ofp_a);
    return 0;
}