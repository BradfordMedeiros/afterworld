#pragma once

#include "../../../ModEngine/src/cscript/cscript_binding.h"
#include "../util.h"
#include "../gameworld/progress.h"
#include "../resources/sound.h"

struct Gem {
	std::string name;
	bool alreadyPickedUp = false;
	bool canPickup = false;
};
struct Breakable {
	bool isBroken = false;
};

void updateGemStyle(objid id);
void addGem(objid id);
void doPickupGem(objid id);
bool isGem(objid id);
void handleGemCollision(int32_t obj1, int32_t obj2);
void updateGemCanPickup(int numBroken);