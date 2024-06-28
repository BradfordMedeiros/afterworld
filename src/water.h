#ifndef MOD_AFTERWORLD_WATER
#define MOD_AFTERWORLD_WATER

#include "../../ModEngine/src/cscript/cscript_binding.h"
#include "./util.h"
#include "./global.h"

struct Water {
	std::map<objid, std::set<objid>> objectsInWater;
};

void onCollisionEnterWater(Water& water, int32_t obj1, int32_t obj2);
void onCollisionExitWater(Water& water, int32_t obj1, int32_t obj2);
void onObjectRemovedWater(Water& water, objid idRemoved);
void onFrameWater(Water& water);

#endif