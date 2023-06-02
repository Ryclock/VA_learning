#ifndef _FLV_H
#define _FLV_H

struct FLV_Header
{
    unsigned char signature[3];
    unsigned char version;
    unsigned char flags;
    unsigned char header_size[4];
};

struct Tag_Header
{
    unsigned char type;
    unsigned char data_size[3];
    unsigned char time_stamp[3];
    unsigned char time_stamp_extension;
    unsigned char stream_ID[3];
};

struct Video_Tag_Header
{
    unsigned char encoding_type : 4;
    unsigned char frame_type : 4;
};

struct Audio_Tag_Header
{
    unsigned char type : 1;
    unsigned char precision : 1;
    unsigned char sampling_rate : 2;
    unsigned char encoding_type : 4;
};

int parse_flv(const char *);

#endif