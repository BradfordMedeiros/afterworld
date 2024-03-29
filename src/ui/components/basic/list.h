#ifndef MOD_AFTERWORLD_COMPONENTS_LIST
#define MOD_AFTERWORLD_COMPONENTS_LIST

#include "../common.h"
#include "./listitem.h"
#include "./layout.h"
#include "../../views/style.h"

struct ListComponentData {
	std::string name;
	std::optional<std::function<void()>> onClick;
};
extern Component listComponent;

Component wrapWithLabel(Component& innerComponent, float padding = 0.f);


#endif