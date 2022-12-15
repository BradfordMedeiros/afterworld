#include "./tags.h"

CScriptBinding tagsBinding(CustomApiBindings& api, const char* name){
	 auto binding = createCScriptBinding(name, api);
	 return binding;
}