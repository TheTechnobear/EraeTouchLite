#include "demo_app.h"

#include <iostream>
#include <unistd.h>

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

void DemoApp::start() {
    api_ = std::make_shared<EraeApi::EraeApi>(device_);
    api_->start();
}

void DemoApp::stop() {
    api_->stop();
}

void DemoApp::process() {
    std::cout << "demo app" << std::endl;
    api_->process();
}
