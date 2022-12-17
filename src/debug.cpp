#include "./debug.h"

bool printKey = false;
CScriptBinding debugBinding(CustomApiBindings& api, const char* name){
	auto binding = createCScriptBinding(name, api);
	auto args = api.getArgs();
	printKey = args.find("printkey") != args.end();
  binding.onKeyCallback = [](int32_t id, void* data, int key, int scancode, int action, int mods) -> void {
   	if (printKey){
   		std::cout << "debugBinding: key = " << key << ", action == " << action << ", scancode = " << scancode << ", mods = " << mods << std::endl;
   	}
  };
	return binding;
}