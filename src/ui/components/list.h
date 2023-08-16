#ifndef MOD_AFTERWORLD_COMPONENTS_LIST
#define MOD_AFTERWORLD_COMPONENTS_LIST

#include "./common.h"
#include "./listitem.h"
#include "./layout.h"

struct ListComponentData {
	std::string name;
	std::optional<std::function<void()>> onClick;
};
extern Component listComponent;


#endif