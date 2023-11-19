#ifndef MOD_AFTERWORLD_MOVEMENT
#define MOD_AFTERWORLD_MOVEMENT

#include <iostream>
#include <vector>
#include "./movementcore.h"

void setActiveEntity(objid id);
std::optional<objid> setNextEntity();
void setEntityTargetLocation(objid id, std::optional<glm::vec3> position);

CScriptBinding movementBinding(CustomApiBindings& api, const char* name);

#endif