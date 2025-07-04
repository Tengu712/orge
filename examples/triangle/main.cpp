#include <iostream>
#include <orge.h>

int main() {
	if (orgeInitializeWith("config.yml")) {
		std::cout << orgeGetErrorMessage() << std::endl;
		return 1;
	}

	while (orgePollEvents()) {
		if (orgeRender()) {
			std::cout << orgeGetErrorMessage() << std::endl;
		}
	}

	orgeTerminate();
	return 0;
}
