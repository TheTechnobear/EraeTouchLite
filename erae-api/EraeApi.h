#pragma once

#include <memory>
#include <vector>


namespace EraeApi {

class EraeApiCallback {
public:
    EraeApiCallback() = default;
    virtual ~EraeApiCallback() = default;

    virtual void onInit() { ; }

    virtual void onDeinit() { ; }

    virtual void onError(unsigned err, const char *errStr) { ; }

    // api
    virtual void onStartTouch(unsigned zone, unsigned touch, float x, float y, float z) { ; }

    virtual void onSlideTouch(unsigned zone, unsigned touch, float x, float y, float z) { ; }

    virtual void onEndTouch(unsigned zone, unsigned touch, float x, float y, float z) { ; }

    virtual void onZoneData(unsigned zone, unsigned width, unsigned height) { ; }

    virtual void onVersion(unsigned version) { ; }

    // midi
    virtual void noteOn(unsigned ch, unsigned n, unsigned v) { ; }

    virtual void noteOff(unsigned ch, unsigned n, unsigned v) { ; }

    virtual void cc(unsigned ch, unsigned cc, unsigned v) { ; }

    virtual void pitchbend(unsigned ch, int v) { ; } // +/- 8192
    virtual void ch_pressure(unsigned ch, unsigned v) { ; }

};

class EraeApiImpl_;

class EraeApi {
public:
    EraeApi(const std::string &device);
    virtual ~EraeApi();
    void start();
    void stop();

    void enableApi();
    void disableApi();
    void requestVersion();
    void requestZoneBoundary(unsigned zone);
    void clearZone(unsigned zone);
    void drawPixel(unsigned zone, unsigned x, unsigned y, unsigned rgb = 0xFFFFFF);
    void drawRectangle(unsigned zone, unsigned x, unsigned y, unsigned w, unsigned h, unsigned rgb = 0xFFFFFF);
    void drawImage(unsigned zone, unsigned x, unsigned y, unsigned w, unsigned h, unsigned *rgb);

    unsigned process(); // call periodically for incoming msgs


    void addCallback(const std::shared_ptr<EraeApiCallback>&);
private:
    EraeApiImpl_ *impl_;
};

}
