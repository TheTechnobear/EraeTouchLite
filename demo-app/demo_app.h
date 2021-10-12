#pragma once

#include <EraeApi.h>
#include <string>

class DemoApp {
public:
    explicit DemoApp(const char* dev) : device_(dev) { }
    void start();
    void stop();
    void process();
private:
    std::string device_;
    std::shared_ptr<EraeApi::EraeApi> api_;
};

