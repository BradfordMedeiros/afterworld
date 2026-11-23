#include "./paths.h"

namespace paths {

#define DEFINE_RESOURCE(name, value, ...) const char* name = value;
#define DEFINE_FILE_RESOURCE(name, value, ...) const char* name = value;
#define DEFINE_DIRECTORY_RESOURCE(name, value, ...) const char* name = value;
#include "./path_defs.h"
#undef DEFINE_RESOURCE
#undef DEFINE_FILE_RESOURCE
#undef DEFINE_DIRECTORY_RESOURCE

std::vector<const char*> allFileResources = {
	#define DEFINE_RESOURCE(name, value, ...)
	#define DEFINE_FILE_RESOURCE(name, value, ...) name,
	#define DEFINE_DIRECTORY_RESOURCE(name, value, ...)
		#include "./path_defs.h"
	#undef DEFINE_RESOURCE
	#undef DEFINE_FILE_RESOURCE
	#undef DEFINE_DIRECTORY_RESOURCE
};

std::vector<const char*> allDirectoryResources = {
	#define DEFINE_RESOURCE(name, value, ...)
	#define DEFINE_FILE_RESOURCE(name, value, ...)
	#define DEFINE_DIRECTORY_RESOURCE(name, value, ...) name,
		#include "./path_defs.h"
	#undef DEFINE_RESOURCE
	#undef DEFINE_FILE_RESOURCE
	#undef DEFINE_DIRECTORY_RESOURCE
};

std::vector<ResourceDefinition> allResourceDefinitions = {
	#define RESOURCE_VALIDATION_TYPE_SELECT(_, validationType, ...) validationType
	#define RESOURCE_VALIDATION_TYPE(...) \
		RESOURCE_VALIDATION_TYPE_SELECT(, ##__VA_ARGS__, "")
	#define DEFINE_RESOURCE(name, value, ...) { name, RESOURCE_VALIDATION_TYPE(__VA_ARGS__) },
	#define DEFINE_FILE_RESOURCE(name, value, ...) { name, RESOURCE_VALIDATION_TYPE(__VA_ARGS__) },
	#define DEFINE_DIRECTORY_RESOURCE(name, value, ...) { name, RESOURCE_VALIDATION_TYPE(__VA_ARGS__) },
		#include "./path_defs.h"
	#undef DEFINE_RESOURCE
	#undef DEFINE_FILE_RESOURCE
	#undef DEFINE_DIRECTORY_RESOURCE
	#undef RESOURCE_VALIDATION_TYPE
	#undef RESOURCE_VALIDATION_TYPE_SELECT
};

}