#include <iostream>
#include <orge.h>

constexpr const char *CONFIG = "\
title: simple\n\
width: 640\n\
height: 480\n\
attachments:\n\
 - id: main-att\n\
   format: render-target\n\
   final-layout: present-src\n\
   clear-value: [0.894, 0.619, 0.38, 1.0]\n\
subpasses:\n\
 - id: main-sp\n\
   outputs:\n\
    - id: main-att\n\
      layout: color-attachment\n\
";

int main() {
	if (!orgeInitialize(CONFIG)) {
		std::cout << orgeGetErrorMessage() << std::endl;
		return 1;
	}

	int count = 0;
	while (orgePollEvents()) {
		if (orgeRender()) {
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
