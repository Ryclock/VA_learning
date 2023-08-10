#pragma once
#pragma commment(libm, "ws2_32.lib")

#include <stdint.h>

#define RTP_VERSION 2
#define RTP_PAYLOAD_TYPE_H264 96
#define RTP_PAYLOAD_TYPE_ACC 97
#define RTP_HEADER_SIZE 12
#define RTP_MAX_PKT_SIZE 1400

struct RTP_Header
{
    uint8_t csrc_len : 4;
    uint8_t extension : 1;
    uint8_t padding : 1;
    uint8_t version : 2;
    uint8_t payload_type : 7;
    uint8_t marker : 1;
    uint16_t seq;
    uint32_t timestamp;
    uint32_t ssrc;
};

struct RTP_Packet
{
    struct RTP_Header rh;
    uint8_t payload[0];
};

void RTP_header_init(struct RTP_Packet *, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint16_t, uint32_t, uint32_t);
int RTP_send_packet_over_TCP(int, struct RTP_Packet *, uint32_t);
int RTP_send_packet_over_UDP(int, const char *, int16_t, struct RTP_Packet *, uint32_t);
