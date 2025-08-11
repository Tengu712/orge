#include <iostream>
#include <orge.h>

int main() {
	if (!orgeInitialize()) {
		std::cout << orgeGetErrorMessage() << std::endl;
		return 1;
	}

	int count = 0;
	while (orgeUpdate()) {
		if (!orgeBeginRender() || !orgeEndRender()) {
			std::cout << orgeGetErrorMessage() << std::endl;
		}

		if (count % 60 == 0) {
			std::cout << count / 60 << std::endl;
		}
		count += 1;
	}

	orgeTerminate();
	return 0;
}
