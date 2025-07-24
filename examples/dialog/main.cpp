#include <orge.h>

int main() {
	orgeShowDialog(static_cast<uint32_t>(ORGE_DIALOG_TYPE_INFORMATION), "my info dialog", "Hello, world!");

	orgeInitialize("");
	orgeShowErrorDialog();

	return 0;
}
