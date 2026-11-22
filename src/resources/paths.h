#ifndef MOD_AFTERWORLD_PATHS
#define MOD_AFTERWORLD_PATHS

#include <vector>

// This is macro magic
// The path_defs.h file calls resource-definition macros when included
// in the header it expands to just the declarations
// In the cpp file we redefine it again and it defines the declaration and values


namespace paths {
  #define DEFINE_RESOURCE(name, value) extern const char* name;
  #define DEFINE_FILE_RESOURCE(name, value) extern const char* name;
    #include "./path_defs.h"
  #undef DEFINE_RESOURCE
  #undef DEFINE_FILE_RESOURCE
  
  extern std::vector<const char*> allFileResources;

}

#endif