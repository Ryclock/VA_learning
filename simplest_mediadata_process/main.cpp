#include "pixel.h"
#include "pcm.h"
#include "h264.h"
#include "aac.h"
#include "flv.h"
#include "udp.h"

int main(int argc, char const *argv[])
{
    // split_yuv_420("./attach/lena_256x256_yuv420p.yuv", 256, 256, 1);
    // split_yuv_444("./attach/lena_256x256_yuv444p.yuv", 256, 256, 1);
    // split_rgb("./attach/cie1931_500x500.rgb", 500, 500, 1);
    // trans_yuv420_gray("./attach/lena_256x256_yuv420p.yuv", 256, 256, 1);
    // trans_yuv420_halfy("./attach/lena_256x256_yuv420p.yuv", 256, 256, 1);
    // trans_yuv420_border("./attach/lena_256x256_yuv420p.yuv", 256, 256, 1, 20);
    // trans_rgb24_to_bmp("./attach/lena_256x256_rgb24.rgb", "./output/from_rgb24.bmp", 256, 256);
    // trans_rgb24_to_yuv420("./attach/lena_256x256_rgb24.rgb", "./output/from_rgb24.yuv", 256, 256, 1);
    // generate_yuv420_graybar("./output/graybar420.yuv", 1080, 720, 0, 255, 10);
    // evaluate_yuv420_PSNRy("./attach/lena_256x256_yuv420p.yuv", "./attach/lena_distort_256x256_yuv420p.yuv", 256, 256, 1);

    // split_pcm_16le("./attach/NocturneNo2inEflat_44.1k_s16le.pcm");
    // trans_pcm16le_half_volume_left("./attach/NocturneNo2inEflat_44.1k_s16le.pcm");
    // trans_pcm16le_double_speed("./attach/NocturneNo2inEflat_44.1k_s16le.pcm");
    // trans_pcm16le_to_pcm8le_unsigned("./attach/NocturneNo2inEflat_44.1k_s16le.pcm");
    // cut_pcm16le_singlechannel("./attach/drum.pcm", 2360, 120);
    // trans_pcmle_to_wav("./attach/NocturneNo2inEflat_44.1k_s16le.pcm", "./output/NocturneNo2inEflat_44.1k_s16le.wav", 2, 44100, 16);

    // parse_h264("./attach/sintel.h264");
    // parse_acc("./attach/nocturne.aac");
    // parse_flv("./attach/cuc_ieschool.flv")
    // parse_udp(8080);
    return 0;
}
