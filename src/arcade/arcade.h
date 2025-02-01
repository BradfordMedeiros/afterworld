#ifndef MOD_AFTERWORLD_ARCADE_ARCADE
#define MOD_AFTERWORLD_ARCADE_ARCADE

#include "../../../ModEngine/src/cscript/cscript_binding.h"
#include "../util.h"
#include "./tennis.h"

enum ArcadeType { ARCADE_TENNIS };
struct ArcadeInstance { 
  ArcadeType type;
  ArcadeInterface* interface;
  std::any data;
  std::optional<objid> textureId;
};

void addArcadeType(objid id, std::string type, std::optional<objid> textureId);
void maybeRemoveArcadeType(objid id);

void onKeyArcade(int key, int scancode, int action, int mod);
void updateArcade();
void drawArcade();

#endif