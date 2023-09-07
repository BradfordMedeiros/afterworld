#ifndef MOD_AFTERWORLD_COMPONENTS_OPTIONS
#define MOD_AFTERWORLD_COMPONENTS_OPTIONS

#include "./list.h"

struct Option {
  const char* name;
  std::optional<std::function<void()>>  onClick;
};
struct Options {
  std::vector<Option> options;
  int selectedIndex;
};

extern const int optionsSymbol;

extern Component options;

#endif