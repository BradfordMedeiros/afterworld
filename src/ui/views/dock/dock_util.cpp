#include "./dock_util.h"

extern DockConfigApi dockConfigApi;

std::optional<glm::vec2> toVec2(std::string& text){
  auto parts = split(text, ' ');
  if (parts.size() > 2){
    return std::nullopt;
  }
  std::vector<float> vecParts;
  for (int i = 0; i < parts.size(); i++){
    float number;
    if (parts.at(i) == "-" || parts.at(i) == " " || parts.at(i) == "" || parts.at(i) == "." || parts.at(i) == "-."){
      vecParts.push_back(0.f);
    }else{
      bool isFloat = maybeParseFloat(parts.at(i), number);
      if (!isFloat){
        return std::nullopt;
      }
      vecParts.push_back(number);      
    }
  }
  glm::vec2 value(0.f, 0.f);
  if (vecParts.size() >= 1){
    value.x = vecParts.at(0);
  }
  if (vecParts.size() >= 2){
    value.y = vecParts.at(1);
  }
  return value;
}
std::optional<glm::vec3> toVec3(std::string& text){
  auto parts = split(text, ' ');
  if (parts.size() > 3){
    return std::nullopt;
  }
  std::vector<float> vecParts;
  for (int i = 0; i < parts.size(); i++){
    float number;
    if (parts.at(i) == "-" || parts.at(i) == " " || parts.at(i) == "" || parts.at(i) == "." || parts.at(i) == "-."){
      vecParts.push_back(0.f);
    }else{
      bool isFloat = maybeParseFloat(parts.at(i), number);
      if (!isFloat){
        return std::nullopt;
      }
      vecParts.push_back(number);      
    }
  }
  glm::vec3 value(0.f, 0.f, 0.f);
  if (vecParts.size() >= 1){
    value.x = vecParts.at(0);
  }
  if (vecParts.size() >= 2){
    value.y = vecParts.at(1);
  }
  if (vecParts.size() >= 3){
    value.z = vecParts.at(2);
  }
  return value;
}
std::optional<glm::vec4> toVec4(std::string& text){
  auto parts = split(text, ' ');
  if (parts.size() > 4){
    return std::nullopt;
  }
  std::vector<float> vecParts;
  for (int i = 0; i < parts.size(); i++){
    float number;
    if (parts.at(i) == "-" || parts.at(i) == " " || parts.at(i) == "" || parts.at(i) == "." || parts.at(i) == "-."){
      vecParts.push_back(0.f);
    }else{
      bool isFloat = maybeParseFloat(parts.at(i), number);
      if (!isFloat){
        return std::nullopt;
      }
      vecParts.push_back(number);      
    }
  }
  glm::vec4 value(0.f, 0.f, 0.f, 0.f);
  if (vecParts.size() >= 1){
    value.x = vecParts.at(0);
  }
  if (vecParts.size() >= 2){
    value.y = vecParts.at(1);
  }
  if (vecParts.size() >= 3){
    value.z = vecParts.at(2);
  }
  if (vecParts.size() >= 4){
    value.w = vecParts.at(3);
  }
  return value;
}

std::optional<float> toNumber(std::string& text){
  float number;
  bool isFloat = maybeParseFloat(text, number);
  if (!isFloat){
    return std::nullopt;
  }
  return number;
}
std::optional<float> toPositiveNumber(std::string& text){
  float number;
  bool isFloat = maybeParseFloat(text, number);
  bool isPositiveNumber = isFloat && number >= 0.f;
  if (!isPositiveNumber){
    return std::nullopt;
  }
  return number;
}
std::optional<int> toInteger(std::string& text){
  if (text == "-"){
    return 0;
  }
  auto asInt = std::atoi(text.c_str());
  auto isInteger = std::to_string(asInt) == text;
  if (!isInteger){
    return std::nullopt;
  }
  return asInt;
}
std::optional<int> toPositiveInteger(std::string& text){
  auto asInt = std::atoi(text.c_str());
  auto isPositiveInt = asInt >= 0 && std::to_string(asInt) == text;
  if (!isPositiveInt){
    return std::nullopt;
  }
  return asInt;
}

std::function<bool()> getIsCheckedWorld(std::string key, std::string attribute, std::string enabledValue, std::string disabledValue){
  return [key, attribute, enabledValue, disabledValue]() -> bool {
    auto value = dockConfigApi.getAttribute(key, attribute);
    auto valueStr = std::get_if<std::string>(&value);
    modassert(valueStr, "getIsCheckedWorld not strValue");
    return *valueStr == enabledValue;
  };
}

std::function<void(bool)> getOnCheckedWorld(std::string key, std::string attribute, std::string enabledValue, std::string disabledValue){
  return [key, attribute, enabledValue, disabledValue](bool checked) -> void {
    dockConfigApi.setAttribute(key, attribute, checked ? enabledValue : disabledValue);
  };
}

std::function<bool()> getIsCheckedGameobj(std::string key, std::string enabledValue, std::string disabledValue){
  return [key, enabledValue, disabledValue]() -> bool {
    auto attr = dockConfigApi.getObjAttr(key);
    if (!attr.has_value()){
      return false;
    }
    auto value = attr.value();
    auto valueStr = std::get_if<std::string>(&value);
    modassert(valueStr, "enabledValue not strValue");
    return *valueStr == enabledValue;
  };
}


std::function<bool()> getIsCheckedGameobj(std::string key){
  return [key]() -> bool {
    auto attr = dockConfigApi.getObjAttr(key);
    if (!attr.has_value()){
      return false;
    }
    auto value = attr.value();
    auto valueBool = std::get_if<bool>(&value);
    return *valueBool;
  };
}

std::function<void(bool)> getOnCheckedGameobj(std::string key){
  return [key](bool checked) -> void {
    dockConfigApi.setObjAttr(key, checked ? true : false);
  };
}


std::function<void(bool)> getOnCheckedGameobj(std::string key, std::string enabledValue, std::string disabledValue){
  return [key, enabledValue, disabledValue](bool checked) -> void {
    dockConfigApi.setObjAttr(key, checked ? enabledValue : disabledValue);
  };
}

std::function<void(float)> getSetFloatGameobj(std::string key){
  return [key](float value) -> void {
    std::cout << "dock - set float gameobj: " << value << std::endl;
    dockConfigApi.setObjAttr(key, value);
  };
}

std::function<float()> getFloatGameobj(std::string key){
  return [key]() -> bool {
    auto attr = dockConfigApi.getObjAttr(key);
    if (!attr.has_value()){
      return 0.f;
    }
    auto value = attr.value();
    auto valueFloat = std::get_if<float>(&value);
    std::cout << "dock 2 - get float gameobj: " << *valueFloat << std::endl;
    modassert(valueFloat, "getFloatGameobj not float");
    return *valueFloat;
  };
}

std::function<void(std::string&)> getStrGameObj(const char* attribute){
  return [attribute](std::string& value) -> void {
    dockConfigApi.setObjAttr(attribute, value);
  };
}

std::function<void(std::string& choice, int)> optionsOnClick(std::string key, std::string attribute, std::vector<AttributeValue> optionValueMapping){
  return [key, attribute, optionValueMapping](std::string& choice, int selectedIndex) -> void {
    dockConfigApi.setAttribute(key, attribute, optionValueMapping.at(selectedIndex));
  };
}
std::function<int()> optionsSelectedIndex(std::string key, std::string attribute, std::vector<AttributeValue> optionValueMapping){
  return [key, attribute, optionValueMapping]() -> int {
    auto attr = dockConfigApi.getAttribute(key, attribute);
    for (int i = 0; i < optionValueMapping.size(); i++){
      auto value = optionValueMapping.at(i);
      bool equal = aboutEqual(optionValueMapping.at(i), attr); 
      //std::cout << "comparing to: " << print(attr) << ", to " << print(value) << ", " << (equal ? "true" : "false") << std::endl;
      if (equal){
        return i;
      }
    }
    modassert(false, "options selected index - invalid");
    return 0;
  };
}

std::function<void(std::string& choice, int)> optionsOnClickObj(std::string attribute, std::vector<AttributeValue> optionValueMapping){
  return [attribute, optionValueMapping](std::string& choice, int selectedIndex) -> void {
    dockConfigApi.setObjAttr(attribute, optionValueMapping.at(selectedIndex));
  };
}
std::function<int()> optionsSelectedIndexObj(std::string attribute, std::vector<AttributeValue> optionValueMapping){
  return [attribute, optionValueMapping]() -> int {
    auto attr = dockConfigApi.getObjAttr(attribute);
    if (!attr.has_value()){
      return false;
    }

    for (int i = 0; i < optionValueMapping.size(); i++){
      auto value = optionValueMapping.at(i);
      bool equal = aboutEqual(optionValueMapping.at(i), attr.value()); 
      //std::cout << "comparing to: " << print(attr) << ", to " << print(value) << ", " << (equal ? "true" : "false") << std::endl;
      if (equal){
        return i;
      }
    }
    return -1;
  };
}