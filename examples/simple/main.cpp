#include <iostream>
#include <orge.hpp>

int main() {
	std::cout << "1" << std::endl;

	if (!orgeInitialize()) {
		std::cout << "failed to initialize orge." << std::endl;
		return 1;
	}

	std::cout << "2" << std::endl;

	orgeTerminate();

	std::cout << "3" << std::endl;
	return 0;
}
