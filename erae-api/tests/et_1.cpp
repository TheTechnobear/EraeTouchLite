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
    void onStartTouch(unsigned zone, unsigned touch, float x, float y, float z) override {
        std::cout << "onStartTouch : zone " << zone << "  touch:  " << touch << " -  " << x << " , " << y << " , " << z << std::endl;
    }

    void onSlideTouch(unsigned zone, unsigned touch, float x, float y, float z) override {
        std::cout << "onSlideTouch : zone " << zone << "  touch:  " << touch << " -  " << x << " , " << y << " , " << z << std::endl;
    }

    void onEndTouch(unsigned zone, unsigned touch, float x, float y, float z) override {
        std::cout << "onEndTouch : zone " << zone << "  touch:  " << touch << " -  " << x << " , " << y << " , " << z << std::endl;
    }

    void onZoneData(unsigned zone, unsigned width, unsigned height) override {
        std::cout << "onZoneData : zone " << zone << " , " << width << " ,  " << height << std::endl;
    }
};


void Test::start() {
    unsigned zone = 1;
    api_ = std::make_shared<EraeApi::EraeApi>(device_);
    api_->addCallback(std::make_shared<TestCallback>());
    api_->start();
    api_->disableApi();
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
    api_->drawPixel(zone, 0, 0, 0xFF0000);
    api_->drawPixel(zone, 1, 0, 0x00FF00);
    api_->drawPixel(zone, 0, 1, 0x0000FF);
    api_->drawPixel(zone, 1, 1, 0xFFFFFF);
    api_->drawRectangle(zone, 0, 5, 5, 10, 0xFF0000);
    unsigned img[4] = {0xFF0000, 0xFFFFFF, 0x00FF00, 0x0000FF};
    api_->drawImage(zone, 10, 5, 2, 2, img);
    while (keepRunning) {
        api_->process();
    }
}


