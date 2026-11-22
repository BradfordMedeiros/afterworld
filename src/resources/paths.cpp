#include "./paths.h"

namespace paths {

#define DEFINE_RESOURCE(name, value) const char* name = value;
#define DEFINE_FILE_RESOURCE(name, value) const char* name = value;
#include "./path_defs.h"
#undef DEFINE_RESOURCE
#undef DEFINE_FILE_RESOURCE

std::vector<const char*> allFileResources = {
	#define DEFINE_RESOURCE(name, value)
	#define DEFINE_FILE_RESOURCE(name, value) name,
		#include "./path_defs.h"
	#undef DEFINE_RESOURCE
	#undef DEFINE_FILE_RESOURCE
};

}