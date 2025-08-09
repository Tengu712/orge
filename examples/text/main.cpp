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
		CHECK(orgeBeginRender());
		CHECK(orgeDraw("PL", "square", 1, 0));
		CHECK(orgeEndRender());
	}

	orgeTerminate();
	return 0;
}
