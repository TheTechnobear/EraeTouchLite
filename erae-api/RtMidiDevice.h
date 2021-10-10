#pragma once

#include <MidiDevice.h>
#include <RtMidi.h>

#include <memory>
#include <vector>

#include <readerwriterqueue.h>

#include <atomic>


namespace EraeApi {


class RtMidiDevice : public MidiDevice {

public:
    RtMidiDevice(unsigned inQueueSize = MidiDevice::MAX_QUEUE_SIZE, unsigned outQueueSize = MidiDevice::MAX_QUEUE_SIZE);
    virtual ~RtMidiDevice();
    bool init(const char *indevice, const char *outdevice, bool virtualOutput = false) override;
    void deinit() override;

protected:

    bool isOutputOpen() override { return (midiOutDevice_ && (virtualOpen_ || midiOutDevice_->isPortOpen())); }

    bool send(const MidiMsg &msg) override;

    std::unique_ptr<RtMidiIn> midiInDevice_;
    std::unique_ptr<RtMidiOut> midiOutDevice_;
};


}
