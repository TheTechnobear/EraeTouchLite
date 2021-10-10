#include <iostream>

#include <erae_api.h>
class Test {
public:
	bool start();
	void stop();
	void process(); 
private:
	EraeApi::EraeApi api_;
};


///////////////////////////////////
int main(int argc, char** argv) {

	Test test;
	if(test.start()) {
		test.process();
		test.stop();
	}
}

///////////////////////////////////////////////


bool Test::start() {
	return api_.connect("");
}

void Test::stop() {
	std::cout << "test app" << std::endl;
}

void Test::process() {

}


