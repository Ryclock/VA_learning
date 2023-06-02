#ifndef _AAC_H
#define _AAC_H

// multi-byte data always pattern byte address first, then pattern bit address in a byte.
// in struct, bit order in a byte is first high then low.
struct ADTS_Header
{
    // 0, Fixed
    unsigned char syncword_high : 8;
    // 1, Fixed
    unsigned char protection_absent : 1;
    unsigned char layer : 2;
    unsigned char ID : 1;
    unsigned char syncword_low : 4;
    // 2, Fixed
    unsigned char channel_configuration_high : 1;
    unsigned char private_bit : 1;
    unsigned char sampling_frequency_index : 4;
    unsigned char profile : 2;
    // 3
    // Variable
    unsigned char acc_frame_length_high : 2;
    unsigned char copyright_identification_start : 1;
    unsigned char copyright_identification_bit : 1;
    // Fixed
    unsigned char home : 1;
    unsigned char original_copy : 1;
    unsigned char channel_configuration_low : 2;
    // 4, Variable
    unsigned char aac_frame_length_middle : 8;
    // 5, Variable
    unsigned char adts_buffer_fullness_high : 5;
    unsigned char aac_frame_length_low : 3;
    // 6, Variable
    unsigned char number_of_raw_data_blocks_in_frame : 2;
    unsigned char adts_buffer_fullness_low : 6;
};

int parse_acc(const char *);

#endif