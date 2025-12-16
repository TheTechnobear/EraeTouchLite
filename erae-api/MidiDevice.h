#pragma once

#include <memory>
#include <vector>

#include <readerwriterqueue.h>

#include <atomic>


namespace EraeApi {

class MidiMsg {
public:
    //Note: MidiMsg does not OWN data
    MidiMsg() {
        data_ = nullptr;
        size_ = 0;
    }

    MidiMsg(unsigned char *data, unsigned sz) {
        data_ = data;
        size_ = sz;
    }

    MidiMsg(const MidiMsg &m) {
        data_ = m.data_;
        size_ = m.size_;
    }

    MidiMsg &operator=(const MidiMsg &m) {
        data_ = m.data_;
        size_ = m.size_;
        return *this;
    }

    bool valid() const { return size_ > 0 && data_ != nullptr; }

    unsigned char byte(unsigned idx) const { return valid() && idx < size_ ? data_[idx] : 0x00; }

    const unsigned char *data() const { return data_; }

    unsigned size() const { return size_; }

    static MidiMsg create(unsigned char status) {
        unsigned size = 1;
        unsigned char *data = new unsigned char[size];
        data[0] = status;
        return MidiMsg(data, size);
    }

    static MidiMsg create(unsigned char status, unsigned char d1) {
        unsigned size = 2;
        unsigned char *data = new unsigned char[size];
        data[0] = status;
        data[1] = d1;
        return MidiMsg(data, size);
    }

    static MidiMsg create(unsigned char status, unsigned char d1, unsigned char d2) {
        unsigned size = 3;
        unsigned char *data = new unsigned char[size];
        data[0] = status;
        data[1] = d1;
        data[2] = d2;
        return MidiMsg(data, size);
    }

    friend class MidiDevice;

private:
    void destroy() {
        delete data_;
        data_ = nullptr;
        size_ = 0;

    }

    unsigned char *data_;
    unsigned size_;
};


class MidiCallback {
public:
    MidiCallback() = default;
    virtual ~MidiCallback() = default;

    virtual void noteOn(unsigned ch, unsigned n, unsigned v) { ; }

    virtual void noteOff(unsigned ch, unsigned n, unsigned v) { ; }

    virtual void cc(unsigned ch, unsigned cc, unsigned v) { ; }

    virtual void pitchbend(unsigned ch, int v) { ; } // +/- 8192
    virtual void ch_pressure(unsigned ch, unsigned v) { ; }

    virtual void process(const MidiMsg &msg);
};


class MidiDevice {
public:
    static constexpr int MAX_QUEUE_SIZE = 512;

    MidiDevice(unsigned inQueueSize = MAX_QUEUE_SIZE, unsigned outQueueSize = MAX_QUEUE_SIZE);
    virtual ~MidiDevice();
    virtual bool init(const char *indevice, const char *outdevice, bool virtualOutput = false);
    virtual bool processIn(MidiCallback &cb);
    virtual bool processOut(unsigned maxMsgs = 0);
    virtual void deinit();
    virtual bool isActive();

    bool sendCC(unsigned ch, unsigned cc, unsigned v) { return queueOutMsg(MidiMsg::create(0xB0 + ch, cc, v)); }

    bool sendNoteOn(unsigned ch, unsigned note, unsigned vel) { return queueOutMsg(MidiMsg::create(int(0x90 + ch), note, vel)); }

    bool sendNoteOff(unsigned ch, unsigned note, unsigned vel) { return queueOutMsg(MidiMsg::create(int(0x80 + ch), note, vel)); }

    bool sendBytes(unsigned char *data, unsigned sz);

    bool queueInMsg(const MidiMsg &msg) { return inQueue_.try_enqueue(msg); }

    bool queueOutMsg(const MidiMsg &msg) { return outQueue_.try_enqueue(msg); }

protected:

    bool nextInMsg(MidiMsg &msg) {
        return inQueue_.try_dequeue(msg);
    }

    bool nextOutMsg(MidiMsg &msg) {
        return outQueue_.try_dequeue(msg);
    }

    virtual bool isOutputOpen() = 0;

    virtual bool send(const MidiMsg &msg) = 0;

    bool active_;
    bool virtualOpen_=false;

    moodycamel::ReaderWriterQueue<MidiMsg> inQueue_;
    moodycamel::ReaderWriterQueue<MidiMsg> outQueue_;
};


}
