#include <iostream>
#include <orge.h>

int main() {
	if (!orgeInitializeWith("config.yml")) {
		std::cout << orgeGetErrorMessage() << std::endl;
		return 1;
	}

	if (!orgeLoadWaveFromFile("cdefgabc", "cdefgabc.wav")) {
		std::cout << orgeGetErrorMessage() << std::endl;
		return 1;
	}
	if (!orgePlayWave("cdefgabc", 0)) {
		std::cout << orgeGetErrorMessage() << std::endl;
		return 1;
	}

	while (orgeUpdate()) {
		if (!orgeBeginRender() || !orgeEndRender()) {
			std::cout << orgeGetErrorMessage() << std::endl;
		}
	}

	orgeTerminate();
	return 0;
}
