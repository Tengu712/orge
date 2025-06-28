#include <iostream>
#include <orge.h>

constexpr const char *CONFIG = "\
title: simple\n\
width: 640\n\
height: 480\n\
attachments:\n\
 - id: main\n\
   format: render-target\n\
   final-layout: present-src\n\
   clear-value: [0.894, 0.619, 0.38, 1.0]\n\
subpasses:\n\
 - id: main\n\
   outputs:\n\
    - id: main\n\
      layout: color-attachment\n\
";

int main() {
	if (const auto e = orgeInitialize(CONFIG); e != 0) {
		std::cout << orgeConvertErrorMessage(e) << std::endl;
		return 1;
	}

	int count = 0;
	while (orgePollEvents()) {
		if (const auto e = orgeRender(); e != 0) {
			std::cout << orgeConvertErrorMessage(e) << std::endl;
		}
		if (count % 60 == 0) {
			std::cout << count / 60 << std::endl;
		}
		count += 1;
	}

	orgeTerminate();

	return 0;
}
