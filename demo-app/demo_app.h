#pragma once

#include <EraeApi.h>

class DemoApp {
public:
    void start();
    void stop();
    void process();
private:
    std::shared_ptr<EraeApi::EraeApi> api_;
};

