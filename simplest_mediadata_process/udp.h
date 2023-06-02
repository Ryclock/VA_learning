#ifndef _UDP_H
#define _UDP_H

// struct UDP_Header
// {
//     unsigned short source_port;
//     unsigned short destination_port;
//     unsigned short length;
//     unsigned short checksum;
// };

struct RTP_Fixed_Header
{
    // 0
    unsigned char csrc_length : 4;
    unsigned char extension : 1;
    unsigned char padding : 1;
    unsigned char version : 2;
    // 1
    unsigned char payload : 7;
    unsigned char marker : 1;
    // 2-3
    unsigned short sequence_number;
    // 4-7
    unsigned int time_stamp;
    // 8-11
    unsigned int ssrc_identifier;
};

struct MPEG_TS_FIXED_Header
{
    // 0
    unsigned char sync_byte;
    // 1
    unsigned char PID_high : 5;
    unsigned char transport_priority : 1;
    unsigned char payload_unit_start_indicator : 1;
    unsigned char transport_error_indicator : 1;
    // 2
    unsigned char PID_low;
    // 3
    unsigned char continuity_counter : 4;
    unsigned char adaption_field_control : 2;
    unsigned char transport_scrambling_control : 2;
};

int parse_udp(int);

#endif