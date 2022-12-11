#include "./bindings.h"

CScriptBinding sampleBindingPlugin(CustomApiBindings& api, const char* name){
  auto binding = createCScriptBinding(name, api);
  binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    std::cout << "custom binding: create basic" << std::endl;
    int* value = new int;
    *value = random();
    return value;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
    int* value = (int*)data;
    delete value;
    std::cout << "custom binding: remove basic" << std::endl;
    api.loadScene("./res/scenes/features/textures/scrolling.p.rawscene", {}, std::nullopt, std::nullopt);
  };
  binding.onFrame = [](int32_t id) -> void {
    std::cout << "on frame called: " << id << std::endl;
  };
  return binding;
}

std::vector<CScriptBinding> getUserBindings(CustomApiBindings& api){
  std::vector<CScriptBinding> bindings;
  //bindings.push_back(sampleBindingPlugin(api, "native/sample-game"));
  std::cout << "get user binding quit" << std::endl;
  return bindings;
} 










