#pragma once

#include <mutex>
#include <orge.h>
#include <unordered_map>

namespace input {

class Input {
private:
	mutable std::mutex _mutex;
	std::unordered_map<OrgeScancode, int32_t> _states;

public:
	void update();

	int32_t getState(OrgeScancode scancode) const;
};

} // namespace input
