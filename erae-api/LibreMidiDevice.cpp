#include "LibreMidiDevice.h"

#include <iostream>
#include <fstream>


#define LOG_0(x) std::cerr << x << std::endl;
#define LOG_1(x) std::cerr << x << std::endl;

bool libremidiFindMidiPortId(unsigned &result, const std::string &portName, bool outputPort) {

    if (outputPort) {
        libremidi::midi_out dev;
        for (unsigned i = 0; i < dev.get_port_count(); i++) {
            if (portName.compare(dev.get_port_name(i)) == 0) {
                result = i;
                return true;
            }
        }
    } else {
        libremidi::midi_in dev;
        for (unsigned i = 0; i < dev.get_port_count(); i++) {
            if (portName.compare(dev.get_port_name(i)) == 0) {
                result = i;
                return true;
            }
        }
    }
    return false;
}

namespace EraeApi {

static constexpr int MAX_QUEUE_SIZE = 128;


////////////////////////////////////////////////
LibreMidiDevice::LibreMidiDevice(unsigned inQueueSize, unsigned outQueueSize) : QMidiDevice(inQueueSize, outQueueSize) {
    LOG_0("Using LibreMidiDevice " << libremidi::get_version());
}

LibreMidiDevice::~LibreMidiDevice() {
}


bool LibreMidiDevice::init(const char *indevice, const char *outdevice, bool virtualOutput) {
//#if __APPLE__
    libremidi::API api = libremidi::API::MACOSX_CORE;
//#else //__LINUX__
//    libremidi::API api = libremidi::API::LINUX_ALSA_RAW;
//#endif

    if (!MidiDevice::init(indevice, outdevice, virtualOutput)) return false;
    if (active_) {
        deinit();
    }
    active_ = false;

    bool found = false;

    if (indevice != nullptr && strlen(indevice) > 0) {

        try {
            midiInDevice_.reset(new libremidi::midi_in(api, "MEC MIDI IN DEVICE"));
        } catch (RtMidiError &error) {
            midiInDevice_.reset();
            LOG_0("LibreMidiDevice RtMidiIn ctor error:" << error.what());
            return false;
        }

        unsigned port;
        if (libremidiFindMidiPortId(port, indevice, false)) {
            try {
                midiInDevice_->open_port(port, "MIDI IN");
                found = true;
                LOG_1("LibreMidiDevice Midi input opened : " << indevice);
            } catch (libremidi::midi_exception &error) {
                LOG_0("LibreMidiDevice Midi input open error:" << error.what());
                midiInDevice_.reset();
                return false;
            }
        } else {
            LOG_0("LibreMidiDevice Input device not found : [" << indevice << "]");
            LOG_0("available devices:");
            for (unsigned i = 0; i < midiInDevice_->get_port_count(); i++) {
                LOG_0("[" << midiInDevice_->get_port_name(i) << "]");
            }
            midiInDevice_.reset();
            return false;
        }
        bool midiSysex = false;
        bool midiTime = true;
        bool midiSense = true;
        midiInDevice_->ignore_types(midiSysex, midiTime, midiSense);

        midiInDevice_->set_callback([this](const libremidi::message &message) {
                                        unsigned sz = message.size();
                                        unsigned char *data = new unsigned char[sz];
                                        for (auto i = 0; i < sz; i++) data[i] = message[i];
                                        MidiMsg msg(data, sz);
                                        if (!queueInMsg(msg)) {
                                            LOG_0("midiCallback unable to queue msg");
                                        }
                                    }
        );
    } //midi input

    if (outdevice != nullptr && strlen(outdevice) > 0) {
        try {
            midiOutDevice_.reset(new libremidi::midi_out(api, "MEC MIDI OUT DEVICE"));
        } catch (RtMidiError &error) {
            midiOutDevice_.reset();
            LOG_0("LibreMidiDevice RtMidiOut ctor error:" << error.what());
            return false;
        }
        if (virtualOutput) {
            try {
                midiOutDevice_->open_virtual_port(outdevice);
                LOG_0("LibreMidiDevice Midi virtual output created : " << outdevice);
                virtualOpen_ = true;
            } catch (RtMidiError &error) {
                LOG_0("RtMidiDevice Midi virtual output create error : " << error.what());
                virtualOpen_ = false;
                midiOutDevice_.reset();
                return false;
            }
        } else {
            found = false;
            unsigned port;
            if (libremidiFindMidiPortId(port, outdevice, true)) {
                try {
                    midiOutDevice_->open_port(port, "MIDI OUT");
                    LOG_0("LibreMidiDevice Midi output opened  :" << outdevice);
                    found = true;
                } catch (libremidi::midi_exception &error) {
                    LOG_0("LibreMidiDevice Midi output create error : " << error.what());
                    midiOutDevice_.reset();
                    return false;
                }
            } else {
                LOG_0("LibreMidiDevice Output device not found : [" << outdevice << "]");
                LOG_0("available devices : ");
                for (unsigned i = 0; i < midiOutDevice_->get_port_count(); i++) {
                    LOG_0("[" << midiOutDevice_->get_port_name(i) << "]");
                }
                midiOutDevice_.reset();
            }
        }
    } // midi output


    active_ = midiInDevice_ || midiOutDevice_;
    LOG_0("LibreMidiDevice::init - complete, active : " << active_);
    return active_;
}


void LibreMidiDevice::deinit() {
    LOG_0("LibreMidiDevice::deinit");
    if (midiInDevice_) midiInDevice_->cancel_callback();
    midiInDevice_.reset();
    active_ = false;
}


bool LibreMidiDevice::send(const MidiMsg &m) {
    if (midiOutDevice_ == nullptr || !isOutputOpen()) return false;

    try {
        midiOutDevice_->send_message(m.data(), m.size());
    } catch (libremidi::midi_exception &error) {
        LOG_0("LibreMidiDevice output write error:" << error.what());
        return false;
    }
    return true;
}


}

