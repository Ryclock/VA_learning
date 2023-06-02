#include <cstdio>
#include <cstring>
#include <winsock2.h>
#include "udp.h"

const int BUF_MAX = 65536;

int parse_udp(int port)
{
    // std::FILE *ofp = std::fopen("./output/udp.txt", "wb+");
    std::FILE *ofp = stdout;
    std::FILE *ofp_ts = std::fopen("./output/udp.ts", "wb+");
    if (!ofp || !ofp_ts)
    {
        std::printf("File Open Error");
        return -1;
    }

    WSADATA wsad;
    if (0 != WSAStartup(MAKEWORD(2, 2), &wsad))
    {
        std::fprintf(ofp, "\nWSA Init Error: %d", WSAGetLastError());
        return -1;
    }
    if (wsad.wVersion != 0x0202)
    {
        std::fprintf(ofp, "\nVersion Support Error: %d", WSAGetLastError());
        WSACleanup();
        return -1;
    }

    SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (INVALID_SOCKET == s)
    {
        std::fprintf(ofp, "\nSocket Init Error: %d", WSAGetLastError());
        WSACleanup();
        return -1;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    // 0.0.0.0 means accept incoming connections on all available network interfaces.
    server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
    if (SOCKET_ERROR == bind(s, (sockaddr *)&server_addr, sizeof(server_addr)))
    {
        std::fprintf(ofp, "\nBind Error: %d", WSAGetLastError());
        closesocket(s);
        WSACleanup();
        return -1;
    }

    sockaddr_in remote_addr;
    int remote_addr_len = sizeof(sockaddr_in);
    char *buf = (char *)malloc(BUF_MAX * sizeof(char));
    if (buf == NULL)
    {
        std::printf("Buf memory bad");
        return -1;
    }

    RTP_Fixed_Header rfh;
    MPEG_TS_FIXED_Header mtfh;
    int cnt = -1;
    std::fprintf(ofp, "\nListening on port %d", port);
    while (true)
    {
        // "strlen" will calculate the length of a string that is terminated with a null character('\0').
        // however, it maybe result in undefined behavior when its paramater is an uninitialized buffer.
        int packet_size = recvfrom(s, buf, BUF_MAX, 0, (sockaddr *)&remote_addr, &remote_addr_len);
        cnt++;
        if (SOCKET_ERROR == packet_size)
        {
            std::fprintf(ofp, "\nReceive Error: %d", WSAGetLastError());
        }
        // no new data available temporarily, or the connection has been closed.
        else if (0 == packet_size)
        {
            std::fprintf(ofp, "\n[UDP Pkt]");
        }
        else
        {
            std::fprintf(ofp, "\nAddr: %s", inet_ntoa(remote_addr.sin_addr));
            std::memcpy(&rfh, buf, sizeof(RTP_Fixed_Header));
            std::fprintf(ofp, "\n[RTP Pkt] %5d| %5d| %10lu| %5u| %5d|",
                         cnt, rfh.payload, ntohl(rfh.time_stamp), ntohs(rfh.sequence_number), packet_size);
            std::fwrite(buf + sizeof(RTP_Fixed_Header), packet_size - sizeof(RTP_Fixed_Header), 1, ofp_ts);
            if (33 != rfh.payload)
            {
                continue;
            }
            for (int i = sizeof(RTP_Fixed_Header); i < packet_size; i = i + 188)
            {
                if (0x47 != buf[i])
                {
                    break;
                }
                std::memcpy(&mtfh, buf + i, sizeof(MPEG_TS_FIXED_Header));
                std::fprintf(ofp, "\n  [MPEGTS Pkt]");
            }
        }
    }

    closesocket(s);
    WSACleanup();
    std::fclose(ofp);
    std::fclose(ofp_ts);
    return 0;
}