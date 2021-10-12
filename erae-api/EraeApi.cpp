#include "EraeApi.h"

#include <iostream>
//#define LOG_0(x)
#define LOG_0(x) std::cout << x << std::endl;
//#define LOG_1(x)
#define LOG_1(x) std::cerr << x << std::endl;


#ifdef USE_LIBRE_MIDI
#include "LibreMidiDevice.h"
using MIDI_TYPE_DEVICE = EraeApi::LibreMidiDevice;
#else

#include "RtMidiDevice.h"

using MIDI_TYPE_DEVICE = EraeApi::RtMidiDevice;
#endif

#include "SysExStream.h"

namespace EraeApi {


class EraeApiMidiCallback : public MidiCallback {
public:
    explicit EraeApiMidiCallback(EraeApiImpl_ *parent) : parent_(parent) {}

    void noteOn(unsigned ch, unsigned n, unsigned v) override;
    void noteOff(unsigned ch, unsigned n, unsigned v) override;
    void cc(unsigned ch, unsigned cc, unsigned v) override;
    void pitchbend(unsigned ch, int v) override;
    void ch_pressure(unsigned ch, unsigned v) override;
    void process(const MidiMsg &msg) override;
    void sysex(const unsigned char *data, unsigned sz);
private:
    EraeApiImpl_ *parent_;
};

constexpr uint8_t RECV_PREFIX[] = {0x00, 0x21, 0x50, 0x00, 0x01, 0x00, 0x01, 0x44};

class EraeApiImpl_ {
public:
    explicit EraeApiImpl_(const std::string &device);

    ~EraeApiImpl_() = default;
    void start();
    void stop();

    unsigned process();

    void addCallback(std::shared_ptr<EraeApiCallback> cb) { callbacks_.push_back(cb); }


    // requests
    void enableApi();
    void disableApi();
    void requestZoneBoundary(unsigned zone);
    void clearZone(unsigned zone);
    void drawPixel(unsigned zone, unsigned x, unsigned y, unsigned rgb);
    void drawRectangle(unsigned zone, unsigned x, unsigned y, unsigned w, unsigned h, unsigned rgb);
    void drawImage(unsigned zone, unsigned x, unsigned y, unsigned w, unsigned h, unsigned *rgb);


private:
    friend class EraeApiMidiCallback;

    // callbacks
    void onStartTouch(unsigned zone, unsigned touch, float x, float y, float z);
    void onSlideTouch(unsigned zone, unsigned touch, float x, float y, float z);
    void onEndTouch(unsigned zone, unsigned touch, float x, float y, float z);
    void onZoneData(unsigned zone, unsigned width, unsigned height);

    void noteOn(unsigned ch, unsigned n, unsigned v);
    void noteOff(unsigned ch, unsigned n, unsigned v);
    void cc(unsigned ch, unsigned cc, unsigned v);
    void pitchbend(unsigned ch, int v);
    void ch_pressure(unsigned ch, unsigned v);

    std::vector<std::shared_ptr<EraeApiCallback>> callbacks_;

    std::string devname_;
    MIDI_TYPE_DEVICE device_;
    EraeApiMidiCallback midiCallback_;
};


//---------------------
EraeApiImpl_::EraeApiImpl_(const std::string &device) : devname_(device), midiCallback_(this) {
}

void EraeApiImpl_::start() {
    device_.init(devname_.c_str(), devname_.c_str());
    for (auto cb: callbacks_) {
        cb->onInit();
    }
}


void EraeApiImpl_::stop() {
    for (auto cb: callbacks_) {
        cb->onDeinit();
    }
    device_.deinit();
}

void EraeApiImpl_::onStartTouch(unsigned zone, unsigned touch, float x, float y, float z) {
    for (auto cb: callbacks_) {
        cb->onStartTouch(zone, touch, x, y, z);
    }
}

void EraeApiImpl_::onSlideTouch(unsigned zone, unsigned touch, float x, float y, float z) {
    for (auto cb: callbacks_) {
        cb->onSlideTouch(zone, touch, x, y, z);
    }
}

void EraeApiImpl_::onEndTouch(unsigned zone, unsigned touch, float x, float y, float z) {
    for (auto cb: callbacks_) {
        cb->onEndTouch(zone, touch, x, y, z);
    }
}

void EraeApiImpl_::onZoneData(unsigned zone, unsigned width, unsigned height) {
    for (auto cb: callbacks_) {
        cb->onZoneData(zone, width, height);
    }
}


void EraeApiImpl_::noteOn(unsigned ch, unsigned n, unsigned v) {
    for (auto cb: callbacks_) {
        cb->noteOn(ch, n, v);
    }
}

void EraeApiImpl_::noteOff(unsigned ch, unsigned n, unsigned v) {
    for (auto cb: callbacks_) {
        cb->noteOff(ch, n, v);
    }
}

void EraeApiImpl_::cc(unsigned ch, unsigned cc, unsigned v) {
    for (auto cb: callbacks_) {
        cb->cc(ch, cc, v);
    }
}

void EraeApiImpl_::pitchbend(unsigned ch, int v) {
    for (auto cb: callbacks_) {
        cb->pitchbend(ch, v);
    }
}

void EraeApiImpl_::ch_pressure(unsigned ch, unsigned v) {
    for (auto cb: callbacks_) {
        cb->ch_pressure(ch, v);
    }
}


void EraeApiImpl_::enableApi() {
    SysExOutputStream sysex(13 + sizeof(RECV_PREFIX) + 1);
    sysex.begin();
    sysex.addHeader(SysExMsgs::E_ENABLE);
    sysex.addData(RECV_PREFIX, sizeof(RECV_PREFIX));
    sysex.end();

    if (sysex.isValid()) {
        device_.sendBytes(sysex.releaseBuffer(), sysex.size());
    } else {
        LOG_0("enableApi() - failed");
    }
}

void EraeApiImpl_::disableApi() {
    SysExOutputStream sysex(13 + 1);
    sysex.begin();
    sysex.addHeader(SysExMsgs::E_DISABLE);
    sysex.end();

    if (sysex.isValid()) {
        device_.sendBytes(sysex.releaseBuffer(), sysex.size());
    } else {
        LOG_0("disableApi() - failed");
    }
}

void EraeApiImpl_::requestZoneBoundary(unsigned zone) {
    SysExOutputStream sysex(13 + 1 + 1);
    sysex.begin();
    sysex.addHeader(SysExMsgs::E_BOUNDARY);
    sysex.addUnsigned7(zone);
    sysex.end();

    if (sysex.isValid()) {
        device_.sendBytes(sysex.releaseBuffer(), sysex.size());
    } else {
        LOG_0("requestZoneBoundary() - failed");
    }
}


void EraeApiImpl_::clearZone(unsigned zone) {
    SysExOutputStream sysex(13 + 1 + 1);
    sysex.begin();
    sysex.addHeader(SysExMsgs::E_CLEAR);
    sysex.addUnsigned7(zone);
    sysex.end();

    if (sysex.isValid()) {
        device_.sendBytes(sysex.releaseBuffer(), sysex.size());
    } else {
        LOG_0("clearZone() - failed");
    }
}

void EraeApiImpl_::drawPixel(unsigned zone, unsigned x, unsigned y, unsigned rgb) {
    SysExOutputStream sysex(13 + 6 + 1);
    sysex.begin();
    sysex.addHeader(SysExMsgs::E_D_PIXEL);
    sysex.addUnsigned7(zone);
    sysex.addUnsigned7(x);
    sysex.addUnsigned7(y);
    sysex.addUnsigned7((rgb & 0xFF0000) >> 16);
    sysex.addUnsigned7((rgb & 0x00FF00) >> 8);
    sysex.addUnsigned7((rgb & 0x0000FF));
    sysex.end();

    if (sysex.isValid()) {
        device_.sendBytes(sysex.releaseBuffer(), sysex.size());
    } else {
        LOG_0("drawPixel() - failed");
    }
}

void EraeApiImpl_::drawRectangle(unsigned zone, unsigned x, unsigned y, unsigned w, unsigned h, unsigned rgb) {
//    assert(sizeof(unsigned) == 4);
    SysExOutputStream sysex(13 + 8 + 1);
    sysex.begin();
    sysex.addHeader(SysExMsgs::E_D_RECT);
    sysex.addUnsigned7(zone);
    sysex.addUnsigned7(x);
    sysex.addUnsigned7(y);
    sysex.addUnsigned7(w);
    sysex.addUnsigned7(h);
    sysex.addUnsigned7((rgb & 0xFF0000) >> 16);
    sysex.addUnsigned7((rgb & 0x00FF00) >> 8);
    sysex.addUnsigned7((rgb & 0x0000FF));
    sysex.end();

    if (sysex.isValid()) {
        device_.sendBytes(sysex.releaseBuffer(), sysex.size());
    } else {
        LOG_0("drawRectangle() - failed");
    }
}


void EraeApiImpl_::drawImage(unsigned zone, unsigned x, unsigned y, unsigned w, unsigned h, unsigned *rgb) {
    static uint8_t *bitbuf7 = nullptr;
    static size_t maxsz7 = 0;
    static uint8_t *bitbuf8 = nullptr;
    static size_t maxsz8 = 0;
    if (bitbuf7 == nullptr) {
        unsigned sz = 42 * 24;
        maxsz8 = sz * 3;
        maxsz7 = SysExOutputStream::bitizedSize(maxsz8);
        bitbuf7 = new uint8_t[maxsz7];
        bitbuf8 = new uint8_t[maxsz8];
    }

    size_t sz = w * h;
    size_t sz8 = sz * 3;
    if (sz8 > maxsz8) {
        LOG_1("drawImage : bitbuf8 too small");
        return;
    }

    for (size_t i = 0; i < sz; i++) {
        bitbuf8[i * 3] = (rgb[i] & 0xFF0000) >> 16;
        bitbuf8[i * 3 + 1] = (rgb[i] & 0x00FF00) >> 8;
        bitbuf8[i * 3 + 2] = (rgb[i] & 0x0000FF);
    }

    size_t sz7 = SysExOutputStream::bitizedSize(sz8);
    if (sz7 > maxsz7) {
        LOG_1("drawImage : bitbuf7 too small");
        return;
    }

    uint8_t chksum = SysExOutputStream::bitize(bitbuf8, sz8, bitbuf7);

    SysExOutputStream sysex(13 + 5 + sz7 + 1 + 1);
    sysex.begin();
    sysex.addHeader(SysExMsgs::E_D_IMG);
    sysex.addUnsigned7(zone);
    sysex.addUnsigned7(x);
    sysex.addUnsigned7(y);
    sysex.addUnsigned7(w);
    sysex.addUnsigned7(h);
    sysex.addData(bitbuf7, sz7);
    sysex.addUnsigned7(chksum);
    sysex.end();

    if (sysex.isValid()) {
        device_.sendBytes(sysex.releaseBuffer(), sysex.size());
    } else {
        LOG_0("drawImage() - failed");
    }
}


unsigned EraeApiImpl_::process(void) {
    unsigned count = 0;
    device_.processIn(midiCallback_);
    device_.processOut();
    return count;
}

void EraeApiMidiCallback::process(const MidiMsg &msg) {
    unsigned status = msg.byte(0);
    if (status == 0xF0) {
        sysex(msg.data(), msg.size());
    } else {
        MidiCallback::process(msg);
    }
}

void EraeApiMidiCallback::sysex(const unsigned char *data, unsigned sz) {
    // inbound sysex processing
    SysExInputStream sysex(data, sz);
    if (sysex.isValid()) {
        bool v = sysex.readHeader(RECV_PREFIX, sizeof(RECV_PREFIX));
        if (v) {
            unsigned dat1 = sysex.readUnsigned7();
            unsigned dat2 = sysex.readUnsigned7();
            if (dat1 == 0x7f) {
                // not fingerstream
                unsigned zone = dat2;
                unsigned w = sysex.readUnsigned7();
                unsigned h = sysex.readUnsigned7();
                parent_->onZoneData(zone, w, h);
//                if (dat2 == 0x01) {
//                    // boundary reply
//                    unsigned zone = sysex.readUnsigned7();
//                    unsigned w = sysex.readUnsigned7();
//                    unsigned h = sysex.readUnsigned7();
//                    parent_->onZoneData(zone, w, h);
//                } else {
//                    LOG_1("sysex:: valid prefix, non finger, but unknown msg" << (unsigned) dat1 << ", " << (unsigned) dat2);
//                }
            } else {
                static uint8_t *bitbuf7 = nullptr;
                static uint8_t *bitbuf8 = nullptr;
                static uint8_t bitsz7 = 14;
                static uint8_t bitsz8 = SysExInputStream::unbitizedSize(bitsz7);
                if (bitbuf7 == nullptr) {
                    bitbuf7 = new uint8_t[bitsz7];
                    bitbuf8 = new uint8_t[bitsz8];
                }

                sysex.readData(bitbuf7, bitsz7);
                uint8_t exp_chksum = sysex.readUnsigned7();
                uint8_t chksum = SysExInputStream::unbitize(bitbuf7, bitsz7, bitbuf8);
                if (exp_chksum != chksum) {
                    LOG_1("sysex:: fingerstream invalid chk " << (unsigned) chksum << " != " << (unsigned) exp_chksum);
                }


                // fingerstream
                unsigned touch = dat1 & 0b0001111;
                unsigned zone = dat2;
                float x = 0, y = 0, z = 0;

                float *float_data = static_cast<float *>(static_cast<void *>(bitbuf8));
                x = float_data[0];
                y = float_data[1];
                z = float_data[2];

                int8_t a = dat1 >> 4;
                switch (a) {
                    case 0 :
                        parent_->onStartTouch(zone, touch, x, y, z);
                        break;
                    case 1 :
                        parent_->onSlideTouch(zone, touch, x, y, z);
                        break;
                    case 2 :
                        parent_->onEndTouch(zone, touch, x, y, z);
                        break;
                    default:
                        break;
                }
            }


        } else {
//            LOG_1("sysex:: not from this lib");
        }
    } else {
        LOG_1("sysex:: invalid sysex");
    }
}


void EraeApiMidiCallback::noteOn(unsigned ch, unsigned n, unsigned v) {
    parent_->noteOn(ch, n, v);
}

void EraeApiMidiCallback::noteOff(unsigned ch, unsigned n, unsigned v) {
    parent_->noteOff(ch, n, v);
}

void EraeApiMidiCallback::cc(unsigned ch, unsigned cc, unsigned v) {
    parent_->cc(ch, cc, v);
}

void EraeApiMidiCallback::pitchbend(unsigned ch, int v) {
    parent_->pitchbend(ch, v);
}

void EraeApiMidiCallback::ch_pressure(unsigned ch, unsigned v) {
    parent_->ch_pressure(ch, v);
}


//---------------------

EraeApi::EraeApi(const std::string &d) : impl_(new EraeApiImpl_(d)) {
}

EraeApi::~EraeApi() {
    delete impl_;
    impl_ = nullptr;
}

void EraeApi::start() {
    impl_->start();
}

void EraeApi::stop() {
    impl_->stop();
}


void EraeApi::enableApi() {
    impl_->enableApi();
}

void EraeApi::disableApi() {
    impl_->disableApi();
}

void EraeApi::requestZoneBoundary(unsigned zone) {
    impl_->requestZoneBoundary(zone);
}

void EraeApi::clearZone(unsigned zone) {
    impl_->clearZone(zone);
}

void EraeApi::drawPixel(unsigned zone, unsigned x, unsigned y, unsigned rgb) {
    impl_->drawPixel(zone, x, y, rgb);
}

void EraeApi::drawRectangle(unsigned zone, unsigned x, unsigned y, unsigned w, unsigned h, unsigned rgb) {
    impl_->drawRectangle(zone, x, y, w, h, rgb);
}

void EraeApi::drawImage(unsigned zone, unsigned x, unsigned y, unsigned w, unsigned h, unsigned *rgb) {
    impl_->drawImage(zone, x, y, w, h, rgb);
}

void EraeApi::addCallback(std::shared_ptr<EraeApiCallback> cb) {
    impl_->addCallback(cb);
}


unsigned EraeApi::process() {
    return impl_->process();
}

} // namespace
