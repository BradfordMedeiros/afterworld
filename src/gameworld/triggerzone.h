#ifndef MOD_AFTERWORLD_TRIGGERZONE
#define MOD_AFTERWORLD_TRIGGERZONE

#include "../../../ModEngine/src/cscript/cscript_binding.h"
#include "../util.h"

void addToTriggerZone(objid id, objid triggerZone);

void handleOnTriggerEnter(objid obj1, objid obj2);
void handleOnTriggerExit(objid obj1, objid obj2);
void handleOnTriggerRemove(objid triggerZone);

void handleTriggerZone(int32_t obj1, int32_t obj2);

#endif