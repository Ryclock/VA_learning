#ifndef _PIXEL_H
#define _PIXEL_H

struct Bitmap_File_Header_Without_bftype
{
    unsigned long bf_size;
    unsigned short bf_reserverd1;
    unsigned short bf_reserverd2;
    unsigned long bf_offset;
};

struct BitMap_Info_Header
{
    long bi_size;
    long bi_width;
    long bi_height;
    short bi_planes;
    short bi_bitcount;
    short bi_compression;
    long bi_image_size;
    long bi_Xpels_permeter;
    long bi_Ypels_permeter;
    long bi_color_used;
    long bi_color_important;
};

int split_yuv_420(const char *, int, int, int);
int split_yuv_444(const char *, int, int, int);
int split_rgb(const char *, int, int, int);
int trans_yuv420_gray(const char *, int, int, int);
int trans_yuv420_halfy(const char *, int, int, int);
int trans_yuv420_border(const char *, int, int, int, int);
int trans_rgb24_to_bmp(const char *, const char *, int, int);
int trans_rgb24_to_yuv420(const char *, const char *, int, int, int);
int generate_yuv420_graybar(const char *, int, int, int, int, int);
int evaluate_yuv420_PSNRy(const char *, const char *, int, int, int);

#endif