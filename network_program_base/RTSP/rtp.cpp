#include <sys/types.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include "rtp.h"

void RTP_header_init(struct RTP_Packet *rp, uint8_t csrc_len, uint8_t extension,
                     uint8_t padding, uint8_t version, uint8_t payload_type, uint8_t marker,
                     uint16_t seq, uint32_t timestamp, uint32_t ssrc)
{
    rp->rh.csrc_len = csrc_len;
    rp->rh.extension = extension;
    rp->rh.padding = padding;
    rp->rh.version = version;
    rp->rh.payload_type = payload_type;
    rp->rh.marker = marker;
    rp->rh.seq = seq;
    rp->rh.timestamp = timestamp;
    rp->rh.ssrc = ssrc;
}

int RTP_send_packet_over_TCP(int client_sock_fd, struct RTP_Packet *rp, uint32_t data_size)
{
    rp->rh.seq = htons(rp->rh.seq);
    rp->rh.timestamp = htonl(rp->rh.timestamp);
    rp->rh.ssrc = htonl(rp->rh.ssrc);

    uint32_t RTP_size = RTP_HEADER_SIZE + data_size;
    char *tmp_buf = (char *)malloc(4 + RTP_size);
    tmp_buf[0] = 0x24;
    tmp_buf[1] = 0x00;
    tmp_buf[2] = (uint8_t)((RTP_size & 0xFF00) >> 8);
    tmp_buf[3] = (uint8_t)(RTP_size & 0xFF);
    memcpy(tmp_buf + 4, (char *)rp, RTP_size);

    int ret = send(client_sock_fd, tmp_buf, 4 + RTP_size, 0);
    rp->rh.seq = ntohs(rp->rh.seq);
    rp->rh.timestamp = ntohl(rp->rh.timestamp);
    rp->rh.ssrc = ntohl(rp->rh.ssrc);

    free(tmp_buf);
    tmp_buf = NULL;
    return ret;
}

int RTP_send_packet_over_UDP(int server_RTP_sock_fd, const char *ip, int16_t port, struct RTP_Packet *rp, uint32_t data_size)
{
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.S_un.S_addr = inet_addr(ip);

    rp->rh.seq = htons(rp->rh.seq);
    rp->rh.timestamp = htonl(rp->rh.timestamp);
    rp->rh.ssrc = htonl(rp->rh.ssrc);

    int ret = sendto(server_RTP_sock_fd, (char *)rp, data_size + RTP_HEADER_SIZE, 0, (struct sockaddr *)&addr, sizeof(addr));
    rp->rh.seq = ntohs(rp->rh.seq);
    rp->rh.timestamp = ntohl(rp->rh.timestamp);
    rp->rh.ssrc = ntohl(rp->rh.ssrc);
    return ret;
}
