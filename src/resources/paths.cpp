#include "./paths.h"

namespace paths {

#define DEFINE_RESOURCE(name, value) const char* name = value;
#include "./path_defs.h"
#undef DEFINE_RESOURCE

std::vector<const char*> allResources = {
	#define DEFINE_RESOURCE(name, value) name,
		#include "./path_defs.h"
	#undef DEFINE_RESOURCE
};

}