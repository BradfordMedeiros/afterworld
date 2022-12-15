#ifndef MOD_AFTERWORLD_TAGS
#define MOD_AFTERWORLD_TAGS

#include <iostream>
#include <vector>
#include "../../ModEngine/src/cscript/cscript_binding.h"


/*
	Todo - this should hook up to be responsible for managing all objects with a tag, and then calling back functions
	for example, if you wanted a attribute then when you collide onto it, it turns it red, can do this as opposed to writing custom script
*/
CScriptBinding tagsBinding(CustomApiBindings& api, const char* name);

#endif 