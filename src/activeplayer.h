#ifndef MOD_AFTERWORLD_ACTIVE_PLAYER
#define MOD_AFTERWORLD_ACTIVE_PLAYER

#include <optional>
#include "./util.h"
#include "./global.h"
#include "./movement/movement.h"
#include "./ai/ai.h"

std::optional<objid> getActivePlayerId();
void setActivePlayer(std::optional<objid> id);
void setActivePlayerNext();
void onActivePlayerRemoved(objid id);
void setTempViewpoint(glm::vec3 position, glm::quat rotation);
void popTempViewpoint();
void printActivePlayer();
void onPlayerFrame();

#endif 