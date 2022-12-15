#include "./dialog.h"

CScriptBinding dialogBinding(CustomApiBindings& api, const char* name){
	 auto binding = createCScriptBinding(name, api);
	 return binding;
}