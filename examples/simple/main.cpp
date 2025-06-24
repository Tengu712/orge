#include <iostream>
#include <orge.h>

int main() {
	if (const auto e = orgeInitialize("simple", 640, 480); e != 0) {
		std::cout << orgeConvertErrorMessage(e) << std::endl;
		return 1;
	}

	int count = 0;
	while (orgePollEvents()) {
		if (count % 60 == 0) {
			std::cout << count / 60 << std::endl;
		}
		count += 1;
	}

	orgeTerminate();

	return 0;
}
