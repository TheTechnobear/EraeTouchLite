#pragma once

#include <QMidiDevice.h>
#include <RtMidi.h>

#include <memory>
#include <vector>

#include <atomic>


namespace EraeApi {


class RtMidiDevice : public QMidiDevice {

public:
    RtMidiDevice(unsigned inQueueSize = QMidiDevice::MAX_QUEUE_SIZE, unsigned outQueueSize = QMidiDevice::MAX_QUEUE_SIZE);
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
