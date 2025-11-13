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


template <typename T>
void getSaveValue(std::string scope, std::string subfield, std::vector<T>& _values, std::vector<std::string>& _names){
  bool success = false;
  auto data = gameapi -> loadFromJsonFile (SAVE_FILE, &success);
  if (!success){
    modassertwarn(false, "load save file failed");
    data = {};
  }
  if (data.find(scope) == data.end()){
    data[scope] = {};
  }
  for (auto& [key, value] : data.at(scope)){
    auto keys = split(key, '|');
    if (keys.size() == 2 && keys.at(1) == subfield){
      auto name = keys.at(0);
      std::cout << "name is: " << name << std::endl;
      T* typeValue = std::get_if<T>(&value);
      if (typeValue){
        _values.push_back(*typeValue);
        _names.push_back(name);
      }
    }
  } 
}

std::vector<BoolValueResult> getSaveBoolValues(std::string scope, std::string key){
  std::vector<bool> values;
  std::vector<std::string> names;
  getSaveValue(scope, key, values, names);
  std::vector<BoolValueResult> results;
  modassert(values.size() == names.size(), "getSaveBoolValues mismatch sizes, programming error");
  for (int i = 0; i < values.size(); i++){
    results.push_back(BoolValueResult {
      .field = names.at(i),
      .value = values.at(i),
    });
  }
  return results;
}