#include "demo_app.h"

#include <iostream>
#include <unistd.h>

static const char *DEVICE = "Erae 2 MIDI";
static volatile bool keepRunning = 1;

void intHandler(int dummy) {
    std::cerr << "intHandler called" << std::endl;
    if (!keepRunning) {
        sleep(1);
        exit(-1);
    }
    keepRunning = false;
}


///////////////////////////////////
int main(int argc, char **argv) {
    signal(SIGINT, intHandler);

    const char *device = DEVICE;
    if (argc > 1) device = argv[1];

    DemoApp app(device);
    app.start();
    while (keepRunning) {
        app.process();
    }
    app.stop();
}

///////////////////////////////////////////////
class DemoCallback : public EraeApi::EraeApiCallback {
public:
    explicit DemoCallback(DemoApp *app) : app_(app) {}

    void onStartTouch(unsigned zone, unsigned touch, float x, float y, float z) {
        app_->onStartTouch(zone, touch, x, y, z);
    }

    void onSlideTouch(unsigned zone, unsigned touch, float x, float y, float z) {
        app_->onSlideTouch(zone, touch, x, y, z);
    }

    void onEndTouch(unsigned zone, unsigned touch, float x, float y, float z) {
        app_->onEndTouch(zone, touch, x, y, z);
    }

    void onZoneData(unsigned zone, unsigned width, unsigned height) {
        app_->onZoneData(zone, width, height);
    }
    void onVersion(unsigned version) {
        app_->onVersion(version);
    }

private:
    DemoApp *app_;
};


void DemoApp::start() {
    api_ = std::make_shared<EraeApi::EraeApi>(device_);
    api_->addCallback(std::make_shared<DemoCallback>(this));
    api_->start();
    api_->disableApi(); // just in case something is already started!
    api_->enableApi();
    api_->requestVersion();
    api_->requestZoneBoundary(zone_);
    api_->clearZone(zone_);
}

void DemoApp::stop() {
    api_->stop();
}

void DemoApp::process() {
    advanceTrails();
    api_->process();
    drawTrails();
    api_->process();
    cleanTrails();
    usleep(100 * 1000);
}


void DemoApp::onVersion(unsigned version) {
    std::cout << "onVersion : version " << version <<  std::endl;
}


void DemoApp::onZoneData(unsigned zone, unsigned width, unsigned height) {
    if (zone_ == zone) {
        zoneWidth_ = width;
        zoneHeight_ = height;
        std::cout << "onZoneData : zone " << zone << " , " << width << " ,  " << height << std::endl;
    }
}

static constexpr unsigned MAXCOLOUR = 12;
static constexpr unsigned colours[MAXCOLOUR] = {
    0x007F00, 0x7F0000, 0x00007F,
    0x4F7F4F, 0x7F4F4F, 0x4F4F7F,
    0x4F5F00, 0x7F4F00, 0x4F2F7F,
    0x7F7F4F, 0x7F7F4F, 0x7F4F7F
};


void DemoApp::onStartTouch(unsigned zone, unsigned touch, float x, float y, float z) {
    if (zone != zone_) return;
    // std::cout << "onStartTouch :  " << zone << " - " << touch << " : " << x << " , " << y << " , " << z << std::endl;
    unsigned c = colours[touch % MAXCOLOUR];
    trails_.push_back(TrailData(x, y, c));

}

void DemoApp::onSlideTouch(unsigned zone, unsigned touch, float x, float y, float z) {
    if (zone != zone_) return;
    // std::cout << "onSlideTouch :  " << zone << " - " << touch << " : " << x << " , " << y << " , " << z << std::endl;

    unsigned nx = (unsigned) x;
    for (auto &t: trails_) {
        if (t.X_ == nx) {
            return;
        }
    }

    unsigned c = colours[touch % MAXCOLOUR];
    trails_.push_back(TrailData(x, y, c));
}

void DemoApp::onEndTouch(unsigned zone, unsigned touch, float x, float y, float z) {
    // std::cout << "onEndTouch :  " << zone << " - " << touch << " : " << x << " , " << y << " , " << z << std::endl;
}

void DemoApp::drawTrails() {
    int8_t step = 0x10;
    for (auto &t: trails_) {
        if (t.Y_ == 0x7f) {
            for (unsigned y = 0; y <= t.startY_; y++) {
                api_->drawPixel(zone_, t.X_, y, 0x000000);
            }
        } else {
            unsigned sy = t.startY_, ey = t.Y_;
            unsigned c = t.colour_;
            for (unsigned y = ey; y <= sy; y++) {
                api_->drawPixel(zone_, t.X_, y, c);

                int8_t r = (c & 0xFF0000) >> 16;
                int8_t g = (c & 0x00FF00) >> 8;
                int8_t b = (c & 0x0000FF);
                if (r > step) r -= step; else r = 0;
                if (g > step) g -= step; else g = 0;
                if (b > step) b -= step; else b = 0;
                c = (r << 16) + (g << 8) + b;
            }
        }
    }
}


void DemoApp::advanceTrails() {
    for (auto &t: trails_) {
        if (t.Y_ == 0) {
            t.Y_ = 0x7f; // schedule to remove
        } else {
            t.Y_--;
        }
    }
}

void DemoApp::cleanTrails() {
    for (auto it = trails_.begin(); it != trails_.end();) {
        if (it->Y_ == 0x7f) {
            it = trails_.erase(it);
        } else {
            ++it;
        }
    }
}