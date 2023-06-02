#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "pcm.h"

int split_pcm_16le(const char *url)
{
    std::FILE *ifp = std::fopen(url, "rb+");
    std::FILE *ofp_l = std::fopen("./output/l.pcm", "wb+");
    std::FILE *ofp_r = std::fopen("./output/r.pcm", "wb+");
    if (!ifp || !ofp_l || !ofp_r)
    {
        std::printf("File Open Error");
        return -1;
    }

    unsigned char *buf = (unsigned char *)std::malloc(4);
    while (!std::feof(ifp))
    {
        std::fread(buf, 4, 1, ifp);
        // sample is first L, then R
        std::fwrite(buf, 2, 1, ofp_l);
        std::fwrite(buf + 2, 2, 1, ofp_r);
    }

    std::free(buf);
    std::fclose(ifp);
    std::fclose(ofp_l);
    std::fclose(ofp_r);
    return 0;
}

int trans_pcm16le_half_volume_left(const char *url)
{
    std::FILE *ifp = std::fopen(url, "rb+");
    std::FILE *ofp = std::fopen("./output/half_volume_left.pcm", "wb+");
    if (!ifp || !ofp)
    {
        std::printf("File Open Error");
        return -1;
    }

    unsigned char *buf = (unsigned char *)std::malloc(4);
    while (!std::feof(ifp))
    {
        std::fread(buf, 4, 1, ifp);
        // "Little Endian" means L in low address
        short *sample = (short *)buf;
        *sample /= 2;
        std::fwrite(buf, 4, 1, ofp);
    }

    std::free(buf);
    std::fclose(ifp);
    std::fclose(ofp);
    return 0;
}

int trans_pcm16le_double_speed(const char *url)
{
    std::FILE *ifp = std::fopen(url, "rb+");
    std::FILE *ofp = std::fopen("./output/double_speed.pcm", "wb+");
    if (!ifp || !ofp)
    {
        std::printf("File Open Error");
        return -1;
    }

    unsigned char *buf = (unsigned char *)std::malloc(4);
    int cnt = 0;
    while (!std::feof(ifp))
    {
        std::fread(buf, 4, 1, ifp);
        // sample gets two point, first one is maybe more important
        if (cnt++ % 2 != 0)
        {
            continue;
        }
        std::fwrite(buf, 4, 1, ofp);
    }

    std::free(buf);
    std::fclose(ifp);
    std::fclose(ofp);
    return 0;
}

int trans_pcm16le_to_pcm8le_unsigned(const char *url)
{
    std::FILE *ifp = std::fopen(url, "rb+");
    std::FILE *ofp = std::fopen("./output/8le.pcm", "wb+");
    if (!ifp || !ofp)
    {
        std::printf("File Open Error");
        return -1;
    }

    unsigned char *buf = (unsigned char *)std::malloc(4);
    while (!std::feof(ifp))
    {
        std::fread(buf, 4, 1, ifp);

        // data in higher address is more important
        unsigned char L8 = (*(char *)(buf + 1)) + 128;
        unsigned char R8 = (*(char *)(buf + 3)) + 128;
        std::fwrite(&L8, 1, 1, ofp);
        std::fwrite(&R8, 1, 1, ofp);
    }

    std::free(buf);
    std::fclose(ifp);
    std::fclose(ofp);
    return 0;
}

int trans_pcmle_to_wav(const char *url_in, const char *url_out, int channel_n = 2, int sample_rate = 44100, int bits = 16)
{
    std::FILE *ifp = std::fopen(url_in, "rb+");
    std::FILE *ofp = std::fopen(url_out, "wb+");
    if (!ifp || !ofp)
    {
        std::printf("File Open Error");
        return -1;
    }

    int buf_size = (channel_n * bits) >> 3;
    Wave_Header wh;
    Wave_FMT wf;
    Chunk_Header wd_ch;
    std::memcpy(wh.ch.id, "RIFF", std::strlen("RIFF"));
    std::memcpy(wh.format, "WAVE", std::strlen("WAVE"));
    std::memcpy(wf.ch.id, "fmt ", std::strlen("fmt "));
    wf.ch.size = sizeof(Wave_FMT) - sizeof(Chunk_Header);
    wf.audio_format = 1;
    wf.channel_n = channel_n;
    wf.sample_rate = sample_rate;
    wf.byte_rate = sample_rate * buf_size;
    wf.block_align = buf_size;
    wf.bits_per_sample = bits;
    std::memcpy(wd_ch.id, "data", std::strlen("data"));
    wd_ch.size = 0;

    std::fseek(ofp, sizeof(Wave_Header) + sizeof(Wave_FMT) + sizeof(Chunk_Header), 0);
    unsigned char *buf = (unsigned char *)std::malloc(buf_size);
    while (!std::feof(ifp))
    {
        std::fread(buf, buf_size, 1, ifp);
        wd_ch.size += buf_size;
        std::fwrite(buf, buf_size, 1, ofp);
    }
    // wave header's chunk size = file size - wave header's chunk header size
    wh.ch.size = wd_ch.size + sizeof(Wave_Header) + sizeof(Wave_FMT);
    std::rewind(ofp);
    std::fwrite(&wh, sizeof(Wave_Header), 1, ofp);
    std::fwrite(&wf, sizeof(Wave_FMT), 1, ofp);
    std::fwrite(&wd_ch, sizeof(Chunk_Header), 1, ofp);

    std::free(buf);
    std::fclose(ifp);
    std::fclose(ofp);
    return 0;
}

int cut_pcm16le_singlechannel(const char *url, int start_n, int dur_n)
{
    std::FILE *ifp = std::fopen(url, "rb+");
    std::FILE *ofp = std::fopen("./output/cut.pcm", "wb+");
    std::FILE *ofp_stat = std::fopen("./output/cut.txt", "wb+");
    if (!ifp || !ofp || !ofp_stat)
    {
        std::printf("File Open Error");
        return -1;
    }

    unsigned char *buf = (unsigned char *)std::malloc(2);
    int cnt = 0;
    while (!std::feof(ifp))
    {
        std::fread(buf, 2, 1, ifp);
        if (cnt < start_n || cnt >= start_n + dur_n)
        {
            cnt++;
            continue;
        }
        short *sample = (short *)buf;
        std::fwrite(sample, 2, 1, ofp);
        std::fprintf(ofp_stat, "%d,", *sample);
        if (++cnt % 10 == 0)
        {
            std::fprintf(ofp_stat, "\n");
        }
    }

    std::free(buf);
    std::fclose(ifp);
    std::fclose(ofp);
    std::fclose(ofp_stat);
    return 0;
}