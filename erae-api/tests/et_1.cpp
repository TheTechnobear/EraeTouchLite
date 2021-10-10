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


void Test::start() {
    api_ = std::make_shared<EraeApi::EraeApi>(ET_DEVICE);
    api_->start();
}

void Test::stop() {
    api_->stop();
}

void Test::process() {
    std::cout << "test app" << std::endl;
    api_->process();
}


