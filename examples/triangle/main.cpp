#include <iostream>
#include <orge.h>

#define TRY(n) \
	if (!(n)) { \
		std::cout << orgeGetErrorMessage() << std::endl; \
		return 1; \
	}

int main() {
	TRY(orgeInitialize());
	TRY(orgeLoadMesh("triangle"));

	while (orgeUpdate()) {
		const auto result =
			orgeBeginRender()
			&& orgeDraw("PL", "triangle", 1, 0)
			&& orgeEndRender();
		if (!result) {
			std::cout << orgeGetErrorMessage() << std::endl;
		}
	}

	orgeTerminate();
	return 0;
}
