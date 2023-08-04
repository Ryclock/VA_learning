#include "utils/base64.h"

namespace base64
{
    const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    static inline bool is_base64(unsigned char c)
    {
        return (isalnum(c) || (c == '+') || (c == '/'));
    }

    void encode_data_block(int len, int start_pos, const unsigned char *data, unsigned char *char_array_3)
    {
        for (int j = 0; j < len; j++)
        {
            char_array_3[j] = data[start_pos + j];
        }

        if (len == 3)
        {
            return;
        }

        for (int j = len; j < 3; j++)
        {
            char_array_3[j] = '\0';
        }
    }

    void encode_transform_block(const unsigned char *char_array_3, unsigned char *char_array_4)
    {
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;
    }

    void encode_join_block(int len, const unsigned char *char_array_4, std::string &encoded_str)
    {
        for (int j = 0; j < len; j++)
        {
            encoded_str += base64_chars[char_array_4[j]];
        }

        if (len == 4)
        {
            return;
        }

        for (int j = len; j < 4; j++)
        {
            encoded_str += '=';
        }
    }

    static void encode(unsigned char *data, int size, std::string &encoded_str)
    {
        unsigned char char_array_3[3], char_array_4[4];
        int size_left = size;

        while (size_left > 0)
        {
            encode_data_block(3, size - size_left, data, char_array_3);
            encode_transform_block(char_array_3, char_array_4);
            encode_join_block(4, char_array_4, encoded_str);
            size_left -= 3;
        }

        if (size_left == 0)
        {
            return;
        }

        size_left = size_left + 3;
        encode_data_block(size_left, size - size_left, data, char_array_3);
        encode_transform_block(char_array_3, char_array_4);
        encode_join_block(size_left + 1, char_array_4, encoded_str);
    }

    void decode_data_block(int len, uint8_t *char_array_4)
    {
        for (int j = 0; j < len; j++)
        {
            char_array_4[j] = base64_chars.find(char_array_4[j]);
        }

        if (len == 4)
        {
            return;
        }

        for (int j = len; j < 4; j++)
        {
            char_array_4[j] = base64_chars.find(static_cast<char>(0));
        }
    }

    void decode_transform_block(const uint8_t *char_array_4, uint8_t *char_array_3)
    {
        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
    }

    void decode_join_block(int len, const uint8_t *char_array_3, std::string &decoded_str)
    {
        for (int j = 0; j < len; j++)
        {
            decoded_str += char_array_3[j];
        }
    }

    static void decode(std::string &data, std::string &decoded_str)
    {
        uint8_t char_array_4[4], char_array_3[3];
        int len = data.size();
        int i = 0, pos = 0;

        while (len-- && is_base64(data[pos]))
        {
            char_array_4[i++] = data[pos];
            pos++;
            if (i != 4)
            {
                continue;
            }

            decode_data_block(4, char_array_4);
            decode_transform_block(char_array_4, char_array_3);
            decode_join_block(3, char_array_3, decoded_str);
            i = 0;
        }

        if (i == 0)
        {
            return;
        }

        decode_data_block(i, char_array_4);
        decode_transform_block(char_array_4, char_array_3);
        decode_join_block(i - 1, char_array_3, decoded_str);
    }
}
