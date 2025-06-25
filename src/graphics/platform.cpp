#include "platform.hpp"

#ifdef __APPLE__

#include <array>
#include <cstdlib>
#include <climits>
#include <filesystem>
#include <mach-o/dyld.h>

namespace graphics::platform {

constexpr std::array<const char *const, 1> INSTANCE_EXTENSIONS = {"VK_KHR_portability_enumeration"};
constexpr std::array<const char *const, 1> DEVICE_EXTENSIONS = {"VK_KHR_portability_subset"};

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

vk::InstanceCreateFlags getInstanceCreateFlags() {
	return vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
}

std::span<const char *const> getInstanceExtensions() {
	return std::span(INSTANCE_EXTENSIONS);
}

std::span<const char *const> getDeviceExtensions() {
	return std::span(DEVICE_EXTENSIONS);
}

} // namespace graphics::platform

#else

namespace graphics::platform {

Error initialize() {
	return Error::None;
}

vk::InstanceCreateFlags getInstanceCreateFlags() {
	return vk::InstanceCreateFlags();
}

std::span<const char *const> getInstanceExtensions() {
	return {};
}

std::span<const char *const> getDeviceExtensions() {
	return {};
}

} // namespace graphics::platform

#endif
