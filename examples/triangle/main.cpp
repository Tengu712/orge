#include <iostream>
#include <orge.h>

int main() {
	if (const auto e = orgeInitializeWith("config.yml"); e != 0) {
		std::cout << orgeConvertErrorMessage(e) << std::endl;
		return 1;
	}

	while (orgePollEvents()) {
		if (const auto e = orgeRender(); e != 0) {
			std::cout << orgeConvertErrorMessage(e) << std::endl;
		}
	}

	orgeTerminate();
	return 0;
}
