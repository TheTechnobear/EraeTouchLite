#pragma once

#include <EraeApi.h>
#include <string>

struct TrailData {
    TrailData(unsigned x, unsigned y, unsigned c=0x007F00) : X_(x), startY_(y), Y_(y), colour_(c) {  }
    unsigned X_=0;
    unsigned Y_=0;
    unsigned startY_=0;
    unsigned colour_;
};

class DemoApp {
public:
    explicit DemoApp(const char* dev) : device_(dev) { }
    void start();
    void stop();
    void process();

    void onTouch(unsigned zone, EraeApi::EraeApiCallback::TouchAction a, unsigned touch, float x, float y, float z);
    void onZoneData(unsigned zone, unsigned width, unsigned height);
private:
    void onStartTouch(unsigned touch, float x, float y, float z);
    void onSlideTouch(unsigned touch, float x, float y, float z);
    void onEndTouch(unsigned touch, float x, float y, float z);

    void drawTrails();
    void advanceTrails();
    void cleanTrails();

    std::string device_;
    std::shared_ptr<EraeApi::EraeApi> api_;
    unsigned zone_=1;
    unsigned zoneWidth_=0x7f, zoneHeight_=0x7f;
    std::vector<TrailData> trails_;
};

