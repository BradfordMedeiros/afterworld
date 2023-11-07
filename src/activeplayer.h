#ifndef MOD_AFTERWORLD_ACTIVE_PLAYER
#define MOD_AFTERWORLD_ACTIVE_PLAYER

#include <optional>
#include "./util.h"
#include "./global.h"
#include "./movement/movement.h"

std::optional<objid> getActivePlayerId();
void setActivePlayer(std::optional<objid> id);
void setActivePlayerNext();
void onActivePlayerRemoved(objid id);
void printActivePlayer();
void onPlayerFrame();

#endif 