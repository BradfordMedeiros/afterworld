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
  return [attrFuncs](int32_t _, void* data, int32_t idRemoved) -> void {
    std::cout << "tags: id removed: " << idRemoved << std::endl;
    auto objAttrs = gameapi -> getGameObjectAttr(idRemoved);
    for (auto &attrFunc : attrFuncs){
      if (hasAttribute(objAttrs, attrFunc.attr)){
        attrFunc.fn(data, idRemoved);
      }
    }
  };
}

float randomNumber(float min, float max){
  static std::default_random_engine gen;
  std::uniform_real_distribution<double> distribution(min, max);
  return distribution(gen);
}

int randomNumber(int min, int max){
  static std::default_random_engine gen;
  std::uniform_int_distribution<> distribution(min, max);
  return distribution(gen);
}

int closestHitpoint(std::vector<HitObject>& hitpoints, glm::vec3 playerPos){
  modassert(hitpoints.size() > 0, "hitpoints object is size 0");
  int closestIndex = 0;
  auto minDistance = glm::distance(playerPos, hitpoints.at(0).point);
  for (int i = 1; i < hitpoints.size(); i++){
    auto distance = glm::distance(playerPos, hitpoints.at(i).point); 
    if (distance < minDistance){
      closestIndex = i;
      minDistance = distance;
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

ObjectAttrHandle getAttrHandle(objid id){
  return ObjectAttrHandle {
    .attr = gameapi -> getGameObjectAttr(id),
  };
}
std::optional<glm::vec3> getVec3Attr(ObjectAttrHandle& attrHandle, std::string key){
  auto attrValue = getVec3Attr(attrHandle.attr, key);
  return attrValue; 
}
std::optional<std::string> getStrAttr(ObjectAttrHandle& attrHandle, const char* key){
  auto attrValue = getStrAttr(attrHandle.attr, key);
  return attrValue; 
}

std::optional<std::string> getSingleAttr(objid id, const char* key){
  auto objattr = getAttrHandle(id);
  auto attrValue = getStrAttr(objattr, key);
  return attrValue;
}
std::optional<glm::vec3> getSingleVec3Attr(objid id, const char* key){
  auto objattr = getAttrHandle(id);
  auto attrValue = getVec3Attr(objattr, key);
  return attrValue;
}
std::optional<float> getSingleFloatAttr(objid id, const char* key){
  auto objattr = getAttrHandle(id).attr;
  auto attrValue = getFloatAttr(objattr, key);
  return attrValue;
}




std::string uniqueNameSuffix(){
  return std::to_string(getUniqueObjId());
}

std::optional<std::string> getStrWorldState(const char* object, const char* attribute){
  auto worldStates = gameapi -> getWorldState();
  for (auto &worldState : worldStates){
    if (worldState.object == object && worldState.attribute == attribute){
      auto strValue = std::get_if<std::string>(&worldState.value);
      return *strValue;
    }
  }
  return std::nullopt;
}

std::string defaultEnable = "true";
std::string defaultDisable = "false";
bool toggleWorldStateBoolStr(const char* object, const char* attribute, const char* enabled, const char *disabled){
  auto currState = getStrWorldState(object, attribute);
  //modassert(currState == enabled || currState == disabled, "toggle bool invalid value received");

  std::string enableValue = enabled == NULL ? defaultEnable : enabled;
  std::string disableValue = disabled == NULL ? defaultDisable : disabled;
  auto newState = !(currState == enableValue);
  gameapi -> setWorldState({ 
    ObjectValue {
      .object = std::string(object),
      .attribute = std::string(attribute),
      .value = newState ? enableValue : disableValue,
    }
  });
  return newState;
}

std::function<void()> getToggleWorldStateBoolStr(const char* object, const char* attribute){
  return [object, attribute]() -> void {
    toggleWorldStateBoolStr(object, attribute, NULL, NULL);
  };
}

std::function<void()> getToggleWorldStateBoolStr(const char* object, const char* attribute, const char* enabled, const char *disabled){
  return [object, attribute, enabled, disabled]() -> void {
    toggleWorldStateBoolStr(object, attribute, enabled, disabled);
  };
}

std::function<void()> getToggleWorldStateSetStr(const char* object, const char* attribute, const char* value){
  return [object, attribute, value]() -> void {
    std::cout << "toggle world state set str" << std::endl;
    gameapi -> setWorldState({ 
      ObjectValue {
        .object = std::string(object),
        .attribute = std::string(attribute),
        .value = std::string(value),
      }
    });
  };
}

std::function<void()> getToggleWorldStateSetFloat(const char* object, const char* attribute, float value){
  return [object, attribute, value]() -> void {
    std::cout << "toggle world state set str" << std::endl;
    gameapi -> setWorldState({ 
      ObjectValue {
        .object = std::string(object),
        .attribute = std::string(attribute),
        .value = value,
      }
    });
  };
}

void notYetImplementedAlert(){
  gameapi -> sendNotifyMessage("alert", std::string("functionality not yet implemented"));
}


float musicVolume = 1.f;
float gameplayVolume = 1.f;
void setMusicVolume(float volume){
  musicVolume = volume;
}
void setGameplayVolume(float volume){
  gameplayVolume = volume;
}
float getMusicVolume(){
  return musicVolume;
}
float getGameplayVolume(){
  return gameplayVolume;
}
void playMusicClip(std::string&& clipName, objid sceneId, std::optional<float> volume, std::optional<glm::vec3> position){
  if (!volume.has_value()){
    volume = 1.f;
  }else
  volume = volume.value() * musicVolume;
  gameapi -> playClip(clipName, sceneId, volume, position);
}
void playGameplayClip(std::string&& clipName, objid sceneId, std::optional<float> volume, std::optional<glm::vec3> position){
  if (!volume.has_value()){
    volume = 1.f;
  }
  volume = volume.value() * gameplayVolume;
  gameapi -> playClip(clipName, sceneId, volume, position);
}
void playGameplayClipById(objid id, std::optional<float> volume, std::optional<glm::vec3> position){
  if (!volume.has_value()){
    volume = 1.f;
  }
  volume = volume.value() * gameplayVolume;
  gameapi -> playClipById(id, volume, position);
}

