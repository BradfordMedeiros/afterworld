#ifndef MOD_AFTERWORLD_WEAPON
#define MOD_AFTERWORLD_WEAPON

#include <iostream>
#include <vector>
#include "../../../ModEngine/src/cscript/cscript_binding.h"
#include "../global.h"
#include "../materials.h"
#include "./weaponcore.h"
#include "./weapon_vector.h"
#include "../activeplayer.h"

bool getIsGunZoomed();
void maybeChangeGun(std::string gun);

CScriptBinding weaponBinding(CustomApiBindings& api, const char* name);


#endif