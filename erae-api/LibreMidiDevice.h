#pragma once

#include <QMidiDevice.h>

#include <libremidi/libremidi.hpp>

#include <memory>
#include <vector>


#include <atomic>


namespace EraeApi {


class LibreMidiDevice : public QMidiDevice {

public:
    LibreMidiDevice(unsigned inQueueSize = QMidiDevice::MAX_QUEUE_SIZE, unsigned outQueueSize = QMidiDevice::MAX_QUEUE_SIZE);
    virtual ~LibreMidiDevice();
    bool init(const char *indevice, const char *outdevice, bool virtualOutput = false) override;
    void deinit() override;

protected:

    bool isOutputOpen() override { return (midiOutDevice_ && (virtualOpen_ || midiOutDevice_->is_port_open())); }

    bool send(const MidiMsg &msg) override;

    std::unique_ptr<libremidi::midi_in> midiInDevice_;
    std::unique_ptr<libremidi::midi_out> midiOutDevice_;
};


}
