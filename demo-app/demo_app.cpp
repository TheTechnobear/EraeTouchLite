#include "demo_app.h"

#define ET_DEVICE "test"

#include <iostream>

///////////////////////////////////
int main(int argc, char **argv) {

    DemoApp app;
    app.start();
    app.process();
    app.stop();
}

///////////////////////////////////////////////

void DemoApp::start() {
    api_ = std::make_shared<EraeApi::EraeApi>(ET_DEVICE);
    api_->start();
}

void DemoApp::stop() {
    api_->stop();
}

void DemoApp::process() {
    std::cout << "demo app" << std::endl;
    api_->process();
}
