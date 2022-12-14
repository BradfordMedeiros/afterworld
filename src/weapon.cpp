#include "./weapon.h"

extern CustomApiBindings* gameapi;

CScriptBinding weaponBinding(CustomApiBindings& api, const char* name){
  auto binding = createCScriptBinding(name, api);
  binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
  	return NULL;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
  };
  binding.onFrame = [](int32_t id, void* data) -> void {
  };
  return binding;
}

