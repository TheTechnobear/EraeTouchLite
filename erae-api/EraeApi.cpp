#include "EraeApi.h"

#include <iostream>
#include <unistd.h>
#include <iomanip>

#include <sstream>

#include <readerwriterqueue.h>

#include "EraeApi.h"

#ifdef USE_LIBRE_MIDI
#include "LibreMidiDevice.h"
using MIDI_TYPE_DEVICE = EraeApi::LibreMidiDevice;
#else

#include "RtMidiDevice.h"

using MIDI_TYPE_DEVICE = EraeApi::RtMidiDevice;
#endif


namespace EraeApi {


class EraeApiMidiCallback : public MidiCallback {
public:
    EraeApiMidiCallback(EraeApiImpl_ *parent) : parent_(parent) { ; }

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

class EraeApiImpl_ {
public:
    EraeApiImpl_(const std::string &device);

    ~EraeApiImpl_(void) = default;
    void start(void);
    void stop(void);

    unsigned process(void);

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

    void sendSysEx(unsigned type, const char *data, unsigned len);

    // callbacks
    void onTouch(unsigned zone, unsigned touch, float x, float y, float z);
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

void EraeApiImpl_::sendSysEx(unsigned type, const char *data, unsigned len) {
    //TODO
    unsigned sz = 1 + 3 + 1 + 1 + len + 1;
    unsigned char *midi = new unsigned char[sz];
//    unsigned byte = 0;
//    midi[byte++] = 0xF0;
//
//    for (auto i = 0; i < sizeof(E1_Manufacturer); i++) {
//        midi[byte++] = E1_Manufacturer[i];
//    }
//    midi[byte++] = len > 0 ? E1_R_DATA : E1_R_REQ;
//    midi[byte++] = type;
//
//    for (auto i = 0; i < len; i++) {
//        midi[byte++] = data[i];
//    }
//    midi[byte++] = 0xF7;

    device_.sendBytes(midi, sz);
}

void EraeApiImpl_::onTouch(unsigned zone, unsigned touch, float x, float y, float z) {
    for (auto cb: callbacks_) {
        cb->onTouch(zone, touch, x, y, z);
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
//    sendSysEx(E1_T_CONFIG, json.c_str(), json.length());
}

void EraeApiImpl_::disableApi() {

}

void EraeApiImpl_::requestZoneBoundary(unsigned zone) {

}

void EraeApiImpl_::clearZone(unsigned zone) {

}

void EraeApiImpl_::drawPixel(unsigned zone, unsigned x, unsigned y, unsigned rgb) {

}

void EraeApiImpl_::drawRectangle(unsigned zone, unsigned x, unsigned y, unsigned w, unsigned h, unsigned rgb) {
}


void EraeApiImpl_::drawImage(unsigned zone, unsigned x, unsigned y, unsigned w, unsigned h, unsigned *rgb) {

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
// TODO
//    unsigned idx = 0;
//    unsigned status = data[idx++]; // FO
//    unsigned man[3];
//    man[0] = data[idx++];
//    man[1] = data[idx++];
//    man[2] = data[idx++];
//
//    for (auto i = 0; i < 3; i++) {
//        if (man[i] != E1_Manufacturer[i] ) {
//            std::cerr << "sysex not from electra" << std::hex << man[0] << man[1] << man[2] << std::dec << std::endl;
//            return;
//        }
//    }
//
//    unsigned reqres = data[idx++];
//    // if (reqres != E1_R_DATA && reqres != E1_R_REQ && reqres != E1_R_LOG)) {
//    //     std::cerr << "sysex: invalid msg type " << std::hex << reqres << std::dec << std::endl;
//    // }
//
//    unsigned datatype = data[idx++];
//
//    switch (reqres) {
//    case E1_R_DATA : {
//        unsigned jsonsz = sz - idx - 1;
//        char* json = new char[jsonsz + 1];
//        memcpy(json, data + idx , jsonsz);
//        json[jsonsz] = 0;
//        std::string jsonstr = json;
//        delete [] json;
//
//        switch (datatype) {
//        case E1_T_PRESET_0 : {
//            parent_->onPreset(jsonstr);
//            break;
//        }
//        case E1_T_CONFIG : {
//            parent_->onConfig(jsonstr);
//            break;
//        }
//        case E1_T_INFO : {
//            parent_->onInfo(jsonstr);
//            break;
//        }
//        default: {
//            // parent_->onError("invalid data type");
//            std::cerr << "sysex: invalid data type " << std::hex << datatype << std::dec << std::endl;
//        }
//        }
//
//
//        break;
//    }
//    case E1_R_REQ : {
//        // not handling requests - yet
//        break;
//    }
//    case E1_R_LOG : {
//        // not handling log - yet
//        break;
//    }
//    default: {
//        std::cerr << "sysex: invalid msg type " << std::hex << reqres << std::dec << std::endl;
//        break;
//    }
//    }
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


unsigned EraeApi::process() {
    return impl_->process();
}

} // namespace
