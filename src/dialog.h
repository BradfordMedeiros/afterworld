#ifndef MOD_AFTERWORLD_DIALOG
#define MOD_AFTERWORLD_DIALOG

#include <iostream>
#include <vector>
#include "../../ModEngine/src/cscript/cscript_binding.h"


/*
	Not sure if this actually should be a binding type or not, but system to handle conversations, speach, etc

*/
CScriptBinding dialogBinding(CustomApiBindings& api, const char* name);

#endif 