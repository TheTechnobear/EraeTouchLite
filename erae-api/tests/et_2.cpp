#include <iostream>
#include <string>
#include <unistd.h>

#include <EraeApi.h>

static const char *DEVICE = "Erae Touch";

static volatile bool keepRunning = 1;

void intHandler(int dummy) {
    std::cerr << "intHandler called" << std::endl;
    if (!keepRunning) {
        sleep(1);
        exit(-1);
    }
    keepRunning = false;
}

class Test {
public:
    explicit Test(const char *dev) : device_(dev) {}

    void start();
    void stop();
    void process();
private:
    std::string device_;
    std::shared_ptr<EraeApi::EraeApi> api_;
};


///////////////////////////////////
int main(int argc, char **argv) {
    const char *device = DEVICE;
    if (argc > 1) device = argv[1];
    Test test(device);
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
        std::cout << "onZoneData : zone " << zone << " , " << width << " ,  " << height << std::endl;
    }
};

void Test::start() {
    api_ = std::make_shared<EraeApi::EraeApi>(device_);
    api_->addCallback(std::make_shared<TestCallback>());
    api_->start();
    api_->disableApi();
    api_->enableApi();
    api_->requestZoneBoundary(0);
    api_->requestZoneBoundary(1);
    api_->requestZoneBoundary(2);
    api_->requestZoneBoundary(3);
    api_->requestZoneBoundary(4);
    api_->process();
}

void Test::stop() {
    api_->disableApi();
    api_->process();
    api_->stop();
}

void Test::process() {
    std::cout << "test app" << std::endl;
    {
        unsigned zone = 1;
        api_->clearZone(zone);
        api_->drawPixel(zone, 0, 0, 0xFF0000);
        api_->drawPixel(zone, 1, 0, 0x00FF00);
        api_->drawPixel(zone, 0, 1, 0x0000FF);
        api_->drawPixel(zone, 1, 1, 0xFFFFFF);
    }
    {
        unsigned zone = 2;
        api_->clearZone(zone);
        api_->drawRectangle(zone, 5, 0, 5, 5, 0xFF0000);
    }
    {
        unsigned zone = 3;
        api_->clearZone(zone);
        unsigned img[4] = {0xFF0000, 0xFFFFFF, 0x00FF00, 0x0000FF};
        api_->drawImage(zone, 0, 0, 2, 2, img);
    }
    while (keepRunning) {
        api_->process();
    }
}


