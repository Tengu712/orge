#include <iostream>
#include <orge.h>

int main() {
	if (const auto e = orgeInitialize("simple", 640, 480); e != 0) {
		std::cout << orgeConvertErrorMessage(e) << std::endl;
		return 1;
	}

	std::cin.get();

	orgeTerminate();

	return 0;
}
