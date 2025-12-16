#include "RtMidiDevice.h"

#include <iostream>
#include <fstream>


#define LOG_0(x) std::cerr << x << std::endl;
#define LOG_1(x) std::cerr << x << std::endl;

#ifdef __linux__
#include <alsa/asoundlib.h>
extern unsigned int portInfo(snd_seq_t *seq, snd_seq_port_info_t *pinfo, unsigned int type, int portNumber);
#endif

#ifdef __linux__
#include <alsa/asoundlib.h>





// Imported from RtMidi library.

bool RtMidiFindMidiPortId(unsigned &result, const std::string &portName, bool outputPort) {
    snd_seq_t *seq;
    if (snd_seq_open(&seq, "default", SND_SEQ_OPEN_INPUT, 0) < 0)
        return false;

    result = 0;
    bool success = false;
    const unsigned int type = outputPort ? SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE : SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ;

    snd_seq_addr_t addr;
    if (snd_seq_parse_address(seq, &addr, portName.c_str()) >= 0) {
        snd_seq_port_info_t *info;
        snd_seq_port_info_alloca(&info);
        unsigned count = portInfo(seq, info, type, -1);

        for (unsigned i = 0; i < count; ++i) {
            portInfo(seq, info, type, i);

            if (memcmp(&addr, snd_seq_port_info_get_addr(info), sizeof(addr)) == 0) {
                result = i;
                success = true;
                break;
            }
        }
    }

    snd_seq_close(seq);
    return success;
}

#else

bool RtMidiFindMidiPortId(unsigned &result, const std::string &portName, bool outputPort) {
    RtMidiOut out;
    RtMidiIn in;
    RtMidi &rt = outputPort ? (RtMidi &) out : (RtMidi &) in;

    for (unsigned i = 0; i < rt.getPortCount(); i++) {
        if (portName.compare(rt.getPortName(i)) == 0) {
            result = i;
            return true;
        }
    }
    return false;
}

#endif // __linux__

namespace EraeApi {


////////////////////////////////////////////////
RtMidiDevice::RtMidiDevice(unsigned inQueueSize, unsigned outQueueSize) : QMidiDevice(inQueueSize, outQueueSize) {
    LOG_0("Using rtmidi " << RtMidi::getVersion());

    try {
        midiInDevice_.reset(new RtMidiIn(RtMidi::Api::UNSPECIFIED, "ET MIDI IN DEVICE"));
    } catch (RtMidiError &error) {
        midiInDevice_.reset();
        LOG_0("RtMidiDevice RtMidiIn ctor error:" << error.what());
    }

    try {
        midiOutDevice_.reset(new RtMidiOut(RtMidi::Api::UNSPECIFIED, "ET MIDI OUT DEVICE"));
    } catch (RtMidiError &error) {
        midiOutDevice_.reset();
        LOG_0("RtMidiDevice RtMidiOut ctor error:" << error.what());
    }
}

RtMidiDevice::~RtMidiDevice() {
    midiInDevice_.reset();
    midiOutDevice_.reset();
}


void RtMidiDeviceInCallback(double deltatime, std::vector<unsigned char> *message, void *userData) {
    MidiDevice *self = static_cast<MidiDevice *>(userData);
    unsigned sz = message->size();
    unsigned char *data = new unsigned char[sz];
    memcpy(data, message->data(), sz);

    // std::cerr << "MRtMidiDeviceInCallback " << std::hex << data[0] <<  " - " << message->at(0) << std::endl;
    MidiMsg msg(data, sz);
    if (!self->queueInMsg(msg)) {
        LOG_0("midiCallback unable to queue msg");
    }
}


bool RtMidiDevice::init(const char *indevice, const char *outdevice, bool virtualOutput) {
    if (!MidiDevice::init(indevice, outdevice, virtualOutput)) return false;

    if (active_) {
        deinit();
    }
    active_ = false;
    if (indevice != nullptr && strlen(indevice) > 0) {

        unsigned port;
        if (RtMidiFindMidiPortId(port, indevice, false)) {
            try {
                midiInDevice_->openPort(port, "MIDI IN");
                LOG_1("RtMidiDevice Midi input opened : " << indevice);
            } catch (RtMidiError &error) {
                LOG_0("RtMidiDevice Midi input open error:" << error.what());
                midiInDevice_.reset();
                return false;
            }
        } else {
            LOG_0("RtMidiDevice Input device not found : [" << indevice << "]");
            LOG_0("available devices:");
            for (unsigned i = 0; i < midiInDevice_->getPortCount(); i++) {
                LOG_0("[" << midiInDevice_->getPortName(i) << "]");
            }
            return false;
        }
        bool midiSysex = false;
        bool midiTime = true;
        bool midiSense = true;
        midiInDevice_->ignoreTypes(midiSysex, midiTime, midiSense);
        midiInDevice_->setCallback(RtMidiDeviceInCallback, this);
    } //midi input

    if (outdevice != nullptr && strlen(outdevice) > 0) {
        if (virtualOutput) {
            try {
                midiOutDevice_->openVirtualPort(outdevice);
                LOG_0("RtMidiDevice Midi virtual output created : " << outdevice);
                virtualOpen_ = true;
            } catch (RtMidiError &error) {
                LOG_0("RtMidiDevice Midi virtual output create error : " << error.what());
                virtualOpen_ = false;
                return false;
            }
        } else {
            unsigned port;
            if (RtMidiFindMidiPortId(port, outdevice, true)) {
                try {
                    midiOutDevice_->openPort(port, "MIDI OUT");
                    LOG_0("RtMidiDevice Midi output opened  :" << outdevice);
                } catch (RtMidiError &error) {
                    LOG_0("RtMidiDevice Midi output create error : " << error.what());
                    return false;
                }
            } else {
                LOG_0("RtMidiDevice Output device not found : [" << outdevice << "]");
                LOG_0("available devices : ");
                for (unsigned i = 0; i < midiOutDevice_->getPortCount(); i++) {
                    LOG_0("[" << midiOutDevice_->getPortName(i) << "]");
                }
            }
        }
    } // midi output

    active_ = midiInDevice_ || midiOutDevice_;
    LOG_0("RtMidiIn::init - complete, active : " << active_);
    return active_;
}


void RtMidiDevice::deinit() {
    LOG_0("RtMidiDevice::deinit");
    if (midiInDevice_) midiInDevice_->cancelCallback();
//    midiInDevice_.reset();
    active_ = false;
}


bool RtMidiDevice::send(const MidiMsg &m) {
    if (midiOutDevice_ == nullptr || !isOutputOpen()) return false;

    try {
        midiOutDevice_->sendMessage(m.data(), m.size());
    } catch (RtMidiError &error) {
        LOG_0("RtMidiDevice output write error:" << error.what());
        return false;
    }
    return true;
}


}

