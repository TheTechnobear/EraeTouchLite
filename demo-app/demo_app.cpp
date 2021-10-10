#include "demo_app.h"

#include <iostream>


///////////////////////////////////
int main(int argc, char** argv) {

	DemoApp app;
	if(app.start()) {
		app.process();
		app.stop();
	}
}

///////////////////////////////////////////////


bool DemoApp::start() {
	return api_.connect("");
}

void DemoApp::stop() {
	std::cout << "demo app" << std::endl;
}

void DemoApp::process() {

}
