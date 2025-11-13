#ifndef MOD_AFTERWORLD_SAVE
#define MOD_AFTERWORLD_SAVE

#include "../../../ModEngine/src/cscript/cscript_binding.h"


void persistSave(std::string scope, std::string key, JsonType value);
void persistSaveMap(std::string scope, std::unordered_map<std::string, JsonType>& values);

float getSaveFloatValue(std::string scope, std::string key, float defaultValue);
bool getSaveBoolValue(std::string scope, std::string key, float defaultValue);
std::string getSaveStringValue(std::string scope, std::string key, std::string defaultValue);

struct BoolValueResult {
  std::string field;
  bool value;
};
std::vector<BoolValueResult> getSaveBoolValues(std::string scope, std::string key);

#endif 