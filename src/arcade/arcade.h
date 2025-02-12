#ifndef MOD_AFTERWORLD_ARCADE_ARCADE
#define MOD_AFTERWORLD_ARCADE_ARCADE

#include "../../../ModEngine/src/cscript/cscript_binding.h"
#include "../util.h"
#include "./tennis.h"
#include "./invaders.h"
#include "./helicopter.h"
#include "./interact.h"

enum ArcadeType { ARCADE_TENNIS, ARCADE_INVADERS, ARCADE_INTERACT, ARCADE_HELICOPTER };
struct ArcadeInstance { 
  ArcadeType type;
  ArcadeInterface* interface;
  std::any data;
  std::optional<objid> textureId;
};

void addArcadeType(objid id, std::string type, std::optional<objid> textureId);
void maybeRemoveArcadeType(objid id);
std::optional<objid> arcadeTextureId(objid id);

void onKeyArcade(int key, int scancode, int action, int mod);
void onMouseMoveArcade(double xPos, double yPos, float xNdc, float yNdc);
void onMouseClickArcade(int button, int action, int mods);

void onMessageArcade(std::any& message);

void updateArcade();
void drawArcade();

#endif