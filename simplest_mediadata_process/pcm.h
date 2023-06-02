#ifndef _PCM_H
#define _PCM_H

struct Chunk_Header
{
    char id[4];
    unsigned long size; // chunk size exclude chunk header
};

struct Wave_Header
{
    Chunk_Header ch;
    char format[4];
};

struct Wave_FMT
{
    Chunk_Header ch;
    unsigned short audio_format;
    unsigned short channel_n;
    unsigned long sample_rate;
    unsigned long byte_rate;
    unsigned short block_align;
    unsigned short bits_per_sample;
};

int split_pcm_16le(const char *);
int trans_pcm16le_half_volume_left(const char *);
int trans_pcm16le_double_speed(const char *);
int trans_pcm16le_to_pcm8le_unsigned(const char *);
int cut_pcm16le_singlechannel(const char *, int, int);
int trans_pcmle_to_wav(const char *, const char *, int, int, int);

#endif