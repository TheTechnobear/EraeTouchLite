#pragma once

#include<sstream>

#define LOG_0(x) 
// #define LOG_0(x) std::cout << x 


#define LOG_1(x) 
// #define LOG_1(x) std::cerr << x 


namespace EraeApi {

static constexpr unsigned TB_SYSEX_MSG = 0x20;
static constexpr uint8_t E_Manufacturer[] = {0x00, 0x21, 0x45};

enum SysExMsgs {
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

    void addHeader(unsigned msgtype) {
        *this << E_Manufacturer[0];
        *this << E_Manufacturer[1];
        *this << E_Manufacturer[2];
        *this << TB_SYSEX_MSG;
        *this << msgtype;
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

    bool readHeader() {
        bool bad = false;
        bad |= (read() != 0xF0);
        bad |= (read() != E_Manufacturer[0]);
        bad |= (read() != E_Manufacturer[1]);
        bad |= (read() != E_Manufacturer[2]);
        bad |= (read() != TB_SYSEX_MSG);
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
