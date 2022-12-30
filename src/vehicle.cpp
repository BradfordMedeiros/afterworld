#include "./vehicle.h"

CScriptBinding vehicleBinding(CustomApiBindings& api, const char* name){
  auto binding = createCScriptBinding(name, api);
  return binding;
}
