#include "./util.h"

extern CustomApiBindings* gameapi;

std::string strFromFirstSqlResult(std::vector<std::vector<std::string>>& sqlResult, int index){
  auto value = sqlResult.at(0).at(index);
  return value ;
}
std::string strFromSqlRow(std::vector<std::string>& sqlResult, int index){
  auto value = sqlResult.at(index);
  return value ;
}

float floatFromFirstSqlResult(std::vector<std::vector<std::string>>& sqlResult, int index){
  auto value = sqlResult.at(0).at(index);
  float number = 0.f;
  bool isFloat = maybeParseFloat(value, number);
  modassert(isFloat, "invalid float number");
  return number;
}

int intFromFirstSqlResult(std::vector<std::vector<std::string>>& sqlResult, int index){
  auto value = sqlResult.at(0).at(index);
  return std::atoi(value.c_str());
}

bool boolFromFirstSqlResult(std::vector<std::vector<std::string>>& sqlResult, int index){
  auto value = sqlResult.at(0).at(index);
  modassert(value == "TRUE" || value == "FALSE", "invalid bool value");
  return value == "TRUE";
}

glm::vec3 vec3FromFirstSqlResult(std::vector<std::vector<std::string>>& sqlResult, int xIndex, int yIndex, int zIndex){
  auto xValue = floatFromFirstSqlResult(sqlResult, xIndex);
  auto yValue = floatFromFirstSqlResult(sqlResult, yIndex);
  auto zValue = floatFromFirstSqlResult(sqlResult, zIndex);
  return glm::vec3(xValue, yValue, zValue);
}

glm::vec3 vec3FromFirstSqlResult(std::vector<std::vector<std::string>>& sqlResult, int index){
  return parseVec(sqlResult.at(0).at(index));
}

glm::vec4 vec4FromFirstSqlResult(std::vector<std::vector<std::string>>& sqlResult, int index){
  return parseVec4(sqlResult.at(0).at(index));
}

glm::vec3 vec3FromSqlRow(std::vector<std::string>& sqlRow, int index){
  return parseVec(sqlRow.at(index));
}

glm::quat quatFromFirstSqlResult(std::vector<std::vector<std::string>>& sqlResult, int index){
  auto vec = vec3FromFirstSqlResult(sqlResult, index);
  auto rot = glm::vec4(vec.x, vec.y, vec.z, 0.f);
  return parseQuat(rot);
}


float PI = 3.141592;
float TWO_PI = 2 * PI;
float clampPi(float value){ 
  if (value > 0){
    int numTimes = glm::floor(value / TWO_PI);
    float remain =  value - (TWO_PI * numTimes);
    return (remain > PI) ? (- remain - TWO_PI) : remain;
  }

  int numTimes = glm::floor(value / (-1 * TWO_PI));
  float remain = value + (TWO_PI * numTimes);
  return (remain < (-1 * PI)) ? (remain + TWO_PI) : remain;
}
float limitAngle(float angleRadians, std::optional<float> minAngle, std::optional<float> maxAngle){
  float targetRot = clampPi(angleRadians);
  if (maxAngle.has_value()){
    targetRot = glm::min(maxAngle.value(), targetRot);
  }
  if (minAngle.has_value()){
    targetRot = glm::max(minAngle.value(), targetRot);
  }
  return targetRot;
}

void clickMouse(objid id){
  gameapi ->  click(0, 1); // mouse down
  gameapi -> schedule(id, 5000, NULL, [](void*) -> void {
    gameapi -> click(0, 0);
  });
}


std::function<void(int32_t, void*, int32_t)> getOnAttrAdds(std::vector<AttrFuncValue> attrFuncs){
  return [attrFuncs](int32_t _, void* data, int32_t idAdded) -> void { 
    modlog("health: id added", std::to_string(idAdded));
    auto objAttrs = gameapi -> getGameObjectAttr(idAdded);
    for (auto &attrFunc : attrFuncs){
      auto stringFn = std::get_if<stringAttrFuncValue>(&attrFunc.fn);
      if (stringFn != NULL){
        auto stringValue = getStrAttr(objAttrs, attrFunc.attr);
        if (stringValue.has_value()){
          (*stringFn)(data, idAdded, stringValue.value());
        }
        continue;
      }
      auto floatFn = std::get_if<floatAttrFuncValue>(&attrFunc.fn);
      if (floatFn != NULL){
        auto floatValue = getFloatAttr(objAttrs, attrFunc.attr);
        if (floatValue.has_value()){
          (*floatFn)(data, idAdded, floatValue.value());
        }
        continue;
      }
      auto vec3Fn = std::get_if<vec3AttrFuncValue>(&attrFunc.fn);
      if (vec3Fn != NULL){
        auto vec3Value = getVec3Attr(objAttrs, attrFunc.attr);
        if (vec3Value.has_value()){
          (*vec3Fn)(data, idAdded, vec3Value.value());
        }
        continue;
      }
    }
  };
}


std::function<void(int32_t, void*, int32_t)> getOnAttrRemoved(std::vector<AttrFunc> attrFuncs){
  return [attrFuncs](int32_t _, void* data, int32_t idAdded) -> void { 
    auto objAttrs = gameapi -> getGameObjectAttr(idAdded);
    for (auto &attrFunc : attrFuncs){
      if (hasAttribute(objAttrs, attrFunc.attr)){
        attrFunc.fn(data, idAdded);
      }
    }
  };
}

float randomNumber(float min, float max){
  static std::default_random_engine gen;
  std::uniform_real_distribution<double> distribution(min, max);
  return distribution(gen);
}

int closestHitpoint(std::vector<HitObject>& hitpoints, glm::vec3 playerPos){
  modassert(hitpoints.size() > 0, "hitpoints object is size 0");
  int closestIndex = 0;
  auto minDistance = glm::distance(playerPos, hitpoints.at(0).point);
  for (int i = 1; i < hitpoints.size(); i++){
    if (glm::distance(playerPos, hitpoints.at(i).point) < minDistance){
      closestIndex = i;
    }
  }
  return closestIndex;
}

float hitlength = 2;
void showDebugHitmark(HitObject& hitpoint, objid playerId){
  gameapi -> drawLine(
    hitpoint.point + glm::vec3(0.f, -1.f * hitlength,  0.f),
    hitpoint.point + glm::vec3(0.f, 1.f * hitlength,  0.f),
    true, playerId, std::nullopt,  std::nullopt, std::nullopt
  );
  gameapi -> drawLine(
    hitpoint.point + glm::vec3(-1.f * hitlength, 0.f, 0.f),
    hitpoint.point + glm::vec3(1.f * hitlength, 0.f, 0.f),
    true, playerId, std::nullopt,  std::nullopt, std::nullopt
  );
  gameapi -> drawLine(
    hitpoint.point + glm::vec3(0.f, 0.f, -1.f * hitlength),
    hitpoint.point + glm::vec3(0.f, 0.f, 1.f * hitlength),
    true, playerId, std::nullopt,  std::nullopt, std::nullopt
  );

  gameapi -> drawLine(
    hitpoint.point,
    hitpoint.point + (10.f * (hitpoint.normal * glm::vec3(0.f, 0.f, -1.f))),
    true, playerId, glm::vec4(1.f, 0.f, 0.f, 1.f),  std::nullopt, std::nullopt
  );
}

bool assertDebug = false;
void debugAssertForNow(bool valid, const char* message){
  if (assertDebug){
    modassert(valid, message);
  }
}

std::function<void(int32_t, void*, std::string&, std::any&)> attributeFn(std::function<void(int32_t, void*, std::string&, AttributeValue& value)> fn){
  return [fn](int32_t id, void* data, std::string& key, std::any& anyValue) -> void {
    std::string* value = std::any_cast<std::string>(&anyValue);
    if (value){
      AttributeValue attrValue = *value;
      fn(id, data, key, attrValue);
      return;
    }

    float* floatValue = std::any_cast<float>(&anyValue);
    if (floatValue){
      AttributeValue attrValue = *floatValue;
      fn(id, data, key, attrValue);
      return;
    }
  
    glm::vec3* vec3Value = std::any_cast<glm::vec3>(&anyValue);
    if (vec3Value){
      AttributeValue attrValue = *vec3Value;
      fn(id, data, key, attrValue);
      return;
    }

    glm::vec4* vec4Value = std::any_cast<glm::vec4>(&anyValue);
    if (vec4Value){
      AttributeValue attrValue = *vec4Value;
      fn(id, data, key, attrValue);
      return;
    }

    AttributeValue* attrRawValue = std::any_cast<AttributeValue>(&anyValue);
    if (attrRawValue){
      AttributeValue attrValue = *attrRawValue;
      fn(id, data, key, attrValue);
      return;
    }

    modassert(false, std::string("invalid attribute value: ") + key + ", type = " + anyValue.type().name());
  };
}