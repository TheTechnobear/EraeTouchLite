#pragma once

#include <MidiDevice.h>
#include <readerwriterqueue.h>

namespace EraeApi {

class QMidiDevice : public MidiDevice{
public:
    static constexpr int MAX_QUEUE_SIZE = 512;

    QMidiDevice(unsigned inQueueSize=MAX_QUEUE_SIZE, unsigned outQueueSize=MAX_QUEUE_SIZE) :
        inQueue_(inQueueSize), outQueue_(outQueueSize) {
    }

    bool queueInMsg(const MidiMsg &msg) override { return inQueue_.try_enqueue(msg); }
    bool queueOutMsg(const MidiMsg &msg) override{ return outQueue_.try_enqueue(msg); }
protected: 

    bool nextInMsg(MidiMsg &msg) override { return inQueue_.try_dequeue(msg); }
    bool nextOutMsg(MidiMsg &msg) override { return outQueue_.try_dequeue(msg); }

    moodycamel::ReaderWriterQueue<MidiMsg> inQueue_;
    moodycamel::ReaderWriterQueue<MidiMsg> outQueue_;
};

} //namespace
