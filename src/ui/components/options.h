#ifndef MOD_AFTERWORLD_COMPONENTS_OPTIONS
#define MOD_AFTERWORLD_COMPONENTS_OPTIONS

#include "./common.h"
#include "../../global.h"
#include "./list.h"

struct Option {
  const char* name;
  std::optional<std::function<void()>>  onClick;
};
struct Options {
  std::vector<Option> options;
};

extern const int optionsSymbol;

extern Component options;

#endif