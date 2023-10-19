#ifndef MOD_AFTERWORLD_ACTIVE_PLAYER
#define MOD_AFTERWORLD_ACTIVE_PLAYER

#include <optional>
#include "./util.h"
#include "./global.h"

std::optional<objid> getActivePlayerId();
void setActivePlayer(objid id);
void onActivePlayerRemoved(objid id);
void printActivePlayer();
void onPlayerFrame();

#endif 