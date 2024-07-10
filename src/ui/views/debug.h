#ifndef MOD_AFTERWORLD_COMPONENTS_DEBUG
#define MOD_AFTERWORLD_COMPONENTS_DEBUG

#include "../components/common.h"

struct DebugItem {
	std::string text;
	std::function<void()> onClick;
};
typedef std::variant<std::string, DebugItem> DebugConfigType;

struct DebugConfig {
	std::vector<std::vector<DebugConfigType>> data;
};

extern Component debugComponent;

#endif