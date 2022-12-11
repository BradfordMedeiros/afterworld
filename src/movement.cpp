#include "./movement.h"

CScriptBinding movementBinding(CustomApiBindings& api, const char* name){
  auto binding = createCScriptBinding(name, api);
  binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    return NULL;
  };
  binding.onFrame = [](int32_t id) -> void {
  };
  binding.onKeyCallback = [](int32_t id, int key, int scancode, int action, int mods) -> void {
  };

  return binding;
}