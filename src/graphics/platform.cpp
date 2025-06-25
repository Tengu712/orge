#include "platform.hpp"

#ifdef __APPLE__

#include <cstdlib>
#include <climits>
#include <filesystem>
#include <mach-o/dyld.h>

namespace graphics::platform {

Error initialize() {
	// 実行ファイルパス取得
	char exePath[PATH_MAX];
	uint32_t size = sizeof(exePath);
	if (_NSGetExecutablePath(exePath, &size) != 0) {
		return Error::ApplyMoltenVKICD;
	}

	// ICDファイルパス取得
	const auto icdPath = std::filesystem::path(exePath).parent_path() / "lib" / "MoltenVK_icd.json";
	if (!std::filesystem::exists(icdPath)) {
		return Error::ApplyMoltenVKICD;
	}

	// ICDパスを通す
	setenv("VK_ICD_FILENAMES", icdPath.c_str(), 1);
	return Error::None;
}

} // namespace graphics::platform

#else

namespace graphics::platform {

Error initialize() {
	return Error::None;
}

} // namespace graphics::platform

#endif
