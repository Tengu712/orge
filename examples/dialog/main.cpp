#include <orge.h>

int main() {
	orgeShowDialog(static_cast<uint32_t>(ORGE_DIALOG_TYPE_INFORMATION), "my info dialog", "Hello, world!");

	orgeInitialize();
	orgeShowDialog(static_cast<uint32_t>(ORGE_DIALOG_TYPE_ERROR), "error", orgeGetErrorMessage());

	return 0;
}
