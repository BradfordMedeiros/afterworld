#include "./save.h"

extern CustomApiBindings* gameapi;

const char* SAVE_FILE = "../afterworld/data/save/save.json";

void persistSave(std::string scope, std::string key, JsonType value){
  bool success = false;
  auto data = gameapi -> loadFromJsonFile (SAVE_FILE, &success);
  if (!success){
    data = {};
  }
  if (data.find(scope) == data.end()){
    data[scope] = {};
  }
  data.at(scope)[key] = value;
  gameapi -> saveToJsonFile(SAVE_FILE, data);
}

void persistSaveMap(std::string scope, std::unordered_map<std::string, JsonType>& values){
  bool success = false;
  auto data = gameapi -> loadFromJsonFile (SAVE_FILE, &success);
  if (!success){
    data = {};
  }
  data[scope] = values;
  gameapi -> saveToJsonFile(SAVE_FILE, data);	
}




template <typename T>
T getSaveValue(std::string scope, std::string key, T defaultValue){
  bool success = false;

  auto data = gameapi -> loadFromJsonFile(SAVE_FILE, &success);
  if (!success){
    data = {};
  }
  if (data.find(scope) == data.end()){
    data[scope] = {};
  }

  if (data.at(scope).find(key) == data.at(scope).end()){
    return defaultValue;
  }
  
  JsonType value = data.at(scope).at(key);
  T* typeValue = std::get_if<T>(&value);
  if (typeValue){
    return *typeValue;
  }
  return defaultValue;
}

float getSaveFloatValue(std::string scope, std::string key, float defaultValue){
  return getSaveValue<float>(scope, key, defaultValue);
}
bool getSaveBoolValue(std::string scope, std::string key, float defaultValue){
  return getSaveValue<bool>(scope, key, defaultValue);
}
std::string getSaveStringValue(std::string scope, std::string key, std::string defaultValue){
  return getSaveValue<std::string>(scope, key, defaultValue);
}
