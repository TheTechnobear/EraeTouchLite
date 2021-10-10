#pragma once

#include <erae_api.h>

class DemoApp {
public:
	bool start();
	void stop();
	void process(); 
private:
	EraeApi::EraeApi api_;
};

