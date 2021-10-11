#include <iostream>

#include <EraeApi.h>

#define ET_DEVICE "test"

class Test {
public:
    void start();
    void stop();
    void process();
private:
    std::shared_ptr<EraeApi::EraeApi> api_;
};


///////////////////////////////////
int main(int argc, char **argv) {
    Test test;
    test.start();
    test.process();
    test.stop();
}

///////////////////////////////////////////////

class TestCallback : public EraeApi::EraeApiCallback {
public:
    void onTouch(unsigned zone, TouchAction a, unsigned touch, float x, float y, float z) {
        std::cout << "onTouch : zone " << zone << " action " << (int) a << "  touch:  " << touch << " -  " << x << " , " << y << " , " << z
                  << std::endl;
    }

    void onZoneData(unsigned zone, unsigned width, unsigned height) {
        std::cout << "onZoneData : zone" << zone << " , " << width << " ,  " << height << std::endl;
    }
};

void Test::start() {
    unsigned zone = 1;
    api_ = std::make_shared<EraeApi::EraeApi>(ET_DEVICE);
    api_->start();
    api_->enableApi();
    api_->requestZoneBoundary(zone);
    api_->process();
}

void Test::stop() {
    api_->disableApi();
    api_->process();
    api_->stop();
}

void Test::process() {
    std::cout << "test app" << std::endl;
    unsigned zone = 1;
    api_->clearZone(zone);
    api_->drawPixel(zone, 10, 10, 0xFF0000);
    api_->drawPixel(zone, 11, 11, 0x00FF00);
    api_->drawPixel(zone, 11, 11, 0x0000FF);
    api_->drawRectangle(zone, 5, 15, 10, 7, 0xFF0000);
    unsigned img[4] = {0xFF0000, 0xFFFFFF, 0x00FF00, 0x0000FF};
    api_->drawImage(zone, 12, 12, 2, 2, img);
    api_->process();
}


