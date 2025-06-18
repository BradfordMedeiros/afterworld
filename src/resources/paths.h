#ifndef MOD_AFTERWORLD_PATHS
#define MOD_AFTERWORLD_PATHS

#include <vector>

// This is macro magic
// The paths_defs.h file effectively calls DEFINE_RESOURCE when included
// in the header it expands to just the declarations
// In the cpp file we redefine it again and it defines the declaration and values


namespace paths {
  #define DEFINE_RESOURCE(name, value) extern const char* name;
    #include "./path_defs.h"
  #undef DEFINE_RESOURCE
  
  extern std::vector<const char*> allResources;

}

#endif