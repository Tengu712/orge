#include <iostream>
#include <orge.h>

#define TRY(n) \
	if (!(n)) { \
		std::cout << orgeGetErrorMessage() << std::endl; \
		return 1; \
	}

#define CHECK(n) \
	if (!(n)) { \
		std::cout << orgeGetErrorMessage() << std::endl; \
		continue; \
	}

int main() {
	TRY(orgeInitializeWith("config.yml"));
	TRY(orgePutString("font", "歓迎しますorge"));

	while (orgeUpdate()) {
		CHECK(orgePutText("PL", "font", "歓", 0.0f, 0.0f, 32.0f, 0));
		CHECK(orgeBeginRender());
		CHECK(orgeDrawTexts("PL"));
		CHECK(orgeEndRender());
	}

	orgeTerminate();
	return 0;
}
