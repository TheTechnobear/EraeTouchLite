#pragma once

#include<sstream>

#define LOG_0(x)
// #define LOG_0(x) std::cout << x 


#define LOG_1(x)
// #define LOG_1(x) std::cerr << x 


namespace EraeApi {


static constexpr uint8_t E_Id[] = {0x00, 0x21, 0x50, 0x00, 0x01, 0x00, 0x01};
static constexpr uint8_t E_Midi_Id = 0x01; // currently, always 1
static constexpr uint8_t E_Service = 0x01; // touch service
static constexpr uint8_t E_API = 0x04; // api

// FO  E_ID E_MIDI_ID E_SERVICE E_API E_API_MSG data F7

// example requests
// enable :     0xF0 0x00 0x21 0x50 0x00 0x01 0x00 0x01 0x01 0x01 0x04 0x01 RECEIVER PREFIX BYTES 0xF7
// disable:     0xF0 0x00 0x21 0x50 0x00 0x01 0x00 0x01 0x01 0x01 0x04 0x02 0xF7
// boundary     0xF0 0x00 0x21 0x50 0x00 0x01 0x00 0x01 0x01 0x01 0x04 0x10 ZONE 0xF7
// clear:       0xF0 0x00 0x21 0x50 0x00 0x01 0x00 0x01 0x01 0x01 0x04 0x20 ZONE 0xF7
// d pixel:     0xF0 0x00 0x21 0x50 0x00 0x01 0x00 0x01 0x01 0x01 0x04 0x21 ZONE XPOS YPOS RED GREEN BLUE 0xF7
// d rect:      0xF0 0x00 0x21 0x50 0x00 0x01 0x00 0x01 0x01 0x01 0x04 0x22 ZONE XPOS YPOS WIDTH HEIGHT RED GREEN BLUE 0xF7
// d image:     0xF0 0x00 0x21 0x50 0x00 0x01 0x00 0x01 0x01 0x01 0x04 0x23 ZONE XPOS YPOS WIDTH HEIGHT BIN BIN … BIN CHKS 0xF7

// example replies
// finger:      0xF0 RECEIVER PREFIX BYTES DAT1 DAT2 XYZ1 ... XYZ14 CHKS 0xF7
// boundary rep:0xF0 RECEIVER PREFIX BYTES 0x7F 0x01 ZONE Width Height 0xF7


// finger
// DAT1
// Action type bits aaa (click 0b000 /slide 0b001 /release 0b010) & finger index bits ffff (finger index between 0 and 9) : 0b0aaaffff
// DAT 2 zone
// note: DAT1 cannot be 0x7f, so 0x7f = boundary reply

enum SysExMsgs {
    E_ENABLE = 0x01,
    E_DISABLE = 0x02,
    E_BOUNDARY = 0x10,
    E_CLEAR = 0x20,
    E_D_PIXEL = 0x21,
    E_D_RECT = 0x22,
    E_D_IMG = 0x23,
    E_SYSEX_MAX
};

class SysExOutputStream {
public:
    explicit SysExOutputStream(unsigned max_sz) : size_(0), buf_(new unsigned char[max_sz]), max_sz_(max_sz) { ; }

    ~SysExOutputStream() {
        delete[] buf_;
        buf_ = nullptr;
    }

    SysExOutputStream(SysExOutputStream &) = delete;
    SysExOutputStream &operator=(SysExOutputStream &) = delete;

    SysExOutputStream &operator<<(unsigned b) {
        if (size_ < max_sz_ - 1) {
            buf_[size_++] = b;
        }
        return *this;
    }

    void begin() {
        size_ = 0;
        *this << 0xF0;
    }

    void end() {
        *this << 0xF7;
    }

    bool isValid() {
        return buf_[size_ - 1] == 0xF7;
    }

    unsigned size() const { return size_; }

    unsigned char *buffer() { return buf_; }

    void addHeader(unsigned apimsg) {
        for (auto i = 0; i < sizeof(E_Id); i++) {
            *this << E_Id[i];
        }
        *this << E_Midi_Id;
        *this << E_Service;
        *this << E_API;
        *this << apimsg;
    }

    void addString(const char *str) {
        const char *cstr = str;
        unsigned x = 0;
        while (cstr[x]) {
            *this << ((unsigned) (cstr[x++]) & 0b01111111);
        }
        *this << 0;
    }

    void addUnsigned(unsigned v) {
        unsigned vMSB = (v >> 7) & 0b01111111;
        unsigned vLSB = v & 0b01111111;
        *this << vMSB;
        *this << vLSB;
    }

    void addFloat(float v) {
//        assert(sizeof(float) == 4);
//        assert(sizeof(unsigned) == 4);
        unsigned uval = *static_cast<unsigned *>(static_cast<void *>(&v));
//        unsigned tmp=uval;

        for (unsigned i = 0; i < 5; i++) {
            unsigned bit7 = uval & 0b01111111;
            *this << bit7;
            uval = uval >> 7;
        }
//    LOG_0("addSysExFloat " << tmp << " " << v);
    }

private:
    unsigned char *buf_;
    unsigned size_ = 0;
    unsigned max_sz_ = 0;
};


class SysExInputStream {
public:
    explicit SysExInputStream(const unsigned char *buf, unsigned sz) : buf_(buf), size_(sz), pos_(0) { ; }

    SysExInputStream(SysExOutputStream &) = delete;
    SysExInputStream &operator=(SysExInputStream &) = delete;

    bool isValid() {
        return peek(0) == 0xF0 && peek(size_ - 1) == 0xF7;
    }

    bool atEnd() {
        return (pos_ >= size_) || (peek(pos_) == 0xF7);
    }

    bool readHeader(unsigned *r_prefix, unsigned r_prefix_sz) {
        bool bad = false;
        bad |= (read() != 0xF0);
        for (auto i = 0; i < r_prefix_sz; i++) {
            bad |= (read() != r_prefix[i]);
        }
        return !bad;
    }

    unsigned readUnsigned() {
        unsigned msb = read();
        unsigned lsb = read();
        unsigned id = (msb << 7) + lsb;
        return id;
    }

    std::string readString() {
        std::stringbuf buf;
        bool done = false;
        while (!done) {
            auto ch = read();
            if (ch > 0) {
                buf.sputc(ch);
            } else {
                done = true;
            }
        }
        return buf.str();
    }

    float readFloat() {
        float val = 0.0f;
        unsigned uval = 0;
        for (unsigned i = 0; i < 5; i++) {
            unsigned bit7 = read();
            uval += (bit7 << (i * 7));
        }
        val = *static_cast<float *>(static_cast<void *>(&uval));
        return val;
    }

    unsigned char read() {
        unsigned char v = peek(pos_);
        pos_++;
        return v;
    }

    unsigned pos() { return pos_; }

private:
    unsigned char peek(unsigned pos) {
        if (pos < size_) {
            return buf_[pos];
        }
        LOG_1("SysExInputStream read past end" << pos);
        return 0;
    }

    unsigned pos_ = 0;
    unsigned size_ = 0;
    const unsigned char *buf_;
};

} // namespace
