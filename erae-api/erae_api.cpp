#include "erae_api.h"


#include "sysexstream.h"

namespace EraeApi {

constexpr size_t bitized7size(size_t len);
constexpr size_t unbitized7size(size_t len);
constexpr uint8_t bitize7chksum(const uint8_t *in, size_t inlen, uint8_t *out);
constexpr uint8_t unbitize7chksum(const uint8_t *in, size_t inlen, uint8_t *out);


EraeApi::EraeApi() {

}

EraeApi::~EraeApi() {

}

bool EraeApi::connect(const char *dev) {
    return true;
}



//////////////////////////////////////////////////////////
// example code for converting to/from 7 bit/8bit from API doc

#include <cstdint>
#include <cstddef>

/**
* @brief Get size of the resulting 7 bits bytes array obtained when using the bitize7 function
*/
constexpr size_t bitized7size(size_t len) {
    return len / 7 * 8 + (len % 7 ? 1 + len % 7 : 0);
}

/**
* @brief Get size of the resulting 8 bits bytes array obtained when using the unbitize7 function
*/
constexpr size_t unbitized7size(size_t len) {
    return len / 8 * 7 + (len % 8 ? len % 8 - 1 : 0);
}

/**
* @brief 7-bitize an array of bytes and get the resulting checksum
*
* @param in Input array of 8 bits bytes
* @param inlen Length in bytes of the input array of 8 bits bytes
* @param out An output array of bytes that will receive the 7-bitized bytes
* @return the output 7-bitized bytes XOR checksum
*/
constexpr uint8_t bitize7chksum(const uint8_t *in, size_t inlen, uint8_t *out) {
    uint8_t chksum = 0;
    for (size_t i{0}, outsize{0}; i < inlen; i += 7, outsize += 8) {
        out[outsize] = 0;
        for (size_t j = 0; (j < 7) && (i + j < inlen); ++j) {
            out[outsize] |= (in[i + j] & 0x80) >> (j + 1);
            out[outsize + j + 1] = in[i + j] & 0x7F;
            chksum ^= out[outsize + j + 1];
        }
        chksum ^= out[outsize];
    }
    return chksum;
}

/**
* @brief 7-unbitize an array of bytes and get the incomming checksum
*
* @param in Input array of 7 bits bytes
* @param inlen Length in bytes of the input array of 7 bits bytes
* @param out An output array of bytes that will receive the 7-unbitized bytes
* @return the input 7-bitized bytes XOR checksum
*/
constexpr uint8_t unbitize7chksum(const uint8_t *in, size_t inlen, uint8_t *out) {
    uint8_t chksum = 0;
    for (size_t i{0}, outsize{0}; i < inlen; i += 8, outsize += 7) {
        chksum ^= in[i];
        for (size_t j = 0; (j < 7) && (j + 1 + i < inlen); ++j) {
            out[outsize + j] = ((in[i] << (j + 1)) & 0x80) | in[i + j + 1];
            chksum ^= in[i + j + 1];
        }
    }
    return chksum;
}


}// namespace
