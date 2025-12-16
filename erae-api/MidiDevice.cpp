#include "MidiDevice.h"


#define LOG_0(x)
#define LOG_1(x)

// #include <iostream>
// #define LOG_0(x) std::cerr << x << std::endl;
// #define LOG_1(x) std::cerr << x << std::endl;


namespace EraeApi {


////////////////////////////////////////////////
void MidiCallback::process(const MidiMsg &msg) {
    unsigned status = msg.byte(0);
    unsigned data1 = msg.byte(1);
    unsigned data2 = msg.byte(2);

    unsigned ch = status & 0x0F;
    unsigned type = status & 0xF0;
    switch (type) {
        case 0x90: {
            if (data2 > 0) {
                noteOn(ch, data1, data2);
            } else {
                noteOff(ch, data1, 0);
            }
            break;
        }

        case 0x80: {
            noteOff(ch, data1, data2);
            break;
        }
        case 0xB0: {
            cc(ch, data1, data2);
            break;
        }
        case 0xD0: {
            ch_pressure(ch, data1);
            break;
        }
        case 0xE0: {
            int v = (((data2 << 7) + data1)) - 8192;
            pitchbend(ch, v);
            break;
        }
        default: {
            break;
        }
    } //switch
}

////////////////////////////////////////////////
MidiDevice::MidiDevice() : active_(false){
}

MidiDevice::~MidiDevice() {
    deinit();
}


bool MidiDevice::init(const char *, const char *, bool) {
    return true;
}


bool MidiDevice::processIn(MidiCallback &cb) {
    if (!active_) return false;

    MidiMsg msg;
    while (nextInMsg(msg)) {
        cb.process(msg);
        msg.destroy();
    }
    return true;
}

bool MidiDevice::processOut(unsigned maxMsgs) {
    bool sendMsg = active_;
    MidiMsg msg;
    unsigned mCnt = 0;
    while ((maxMsgs == 0 || mCnt < maxMsgs) && nextOutMsg(msg)) {
        if (sendMsg) {
            sendMsg = send(msg);
            mCnt++;
        }
        msg.destroy();
    }
    return sendMsg;
}

void MidiDevice::deinit() {
}

bool MidiDevice::isActive() {
    return active_;
}

bool MidiDevice::sendCC(unsigned ch, unsigned cc, unsigned v) { 
    return queueOutMsg(MidiMsg::create(0xB0 + ch, cc, v)); 
}

bool MidiDevice::sendNoteOn(unsigned ch, unsigned note, unsigned vel) { 
    return queueOutMsg(MidiMsg::create(int(0x90 + ch), note, vel)); 
}

bool MidiDevice::sendNoteOff(unsigned ch, unsigned note, unsigned vel) { 
    return queueOutMsg(MidiMsg::create(int(0x80 + ch), note, vel)); 
}

bool MidiDevice::sendBytes(unsigned char *data, unsigned sz) {
    // std::cerr << "send SysEx :";
    // for (auto i = 0; i < sz; i++) {
    //     std::cerr << std::hex << data[i];
    // }
    // std::cerr << std::dec << std::endl;
    // delete data;

    return queueOutMsg(MidiMsg(data, sz));
}


}

