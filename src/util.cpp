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
    auto objHandle = getAttrHandle(idAdded);
    for (auto &attrFunc : attrFuncs){
      auto stringFn = std::get_if<stringAttrFuncValue>(&attrFunc.fn);
      if (stringFn != NULL){
        auto attrValue = getAttr(objHandle, attrFunc.attr.c_str());
        auto stringValue = maybeUnwrapAttrOpt<std::string>(attrValue);
        if (stringValue.has_value()){
          (*stringFn)(data, idAdded, stringValue.value());
        }else{
         // modassert(false, std::string("invalid type for: ") + attrFunc.attr);
        }
        continue;
      }
      auto floatFn = std::get_if<floatAttrFuncValue>(&attrFunc.fn);
      if (floatFn != NULL){
        auto attrValue = getAttr(objHandle, attrFunc.attr.c_str());
        auto floatValue = maybeUnwrapAttrOpt<float>(attrValue);
        if (floatValue.has_value()){
          (*floatFn)(data, idAdded, floatValue.value());
        }
        continue;
      }
      auto vec3Fn = std::get_if<vec3AttrFuncValue>(&attrFunc.fn);
      if (vec3Fn != NULL){
        auto attrValue = getAttr(objHandle, attrFunc.attr.c_str());
        auto vec3Value = maybeUnwrapAttrOpt<glm::vec3>(attrValue);
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
    modlog("tags id removed", std::to_string(idRemoved));
    for (auto &attrFunc : attrFuncs){
      if (hasAttribute(idRemoved, attrFunc.attr.c_str())){
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
    .id = id,
  };
}
std::optional<glm::vec2> getVec2Attr(ObjectAttrHandle& attrHandle, std::string key){
  std::optional<glm::vec2*> value = getTypeFromAttr<glm::vec2>(getObjectAttributePtr(attrHandle.id, key.c_str()));
  if (value.has_value()){
   return *(value.value());
  }
  return std::nullopt; 
}
std::optional<glm::vec3> getVec3Attr(ObjectAttrHandle& attrHandle, std::string key){
  std::optional<glm::vec3*> value = getTypeFromAttr<glm::vec3>(getObjectAttributePtr(attrHandle.id, key.c_str()));
  if (value.has_value()){
   return *(value.value());
  }
  return std::nullopt; 
}
std::optional<glm::vec4> getVec4Attr(ObjectAttrHandle& attrHandle, std::string key){
  std::optional<glm::vec4*> value = getTypeFromAttr<glm::vec4>(getObjectAttributePtr(attrHandle.id, key.c_str()));
  if (value.has_value()){
   return *(value.value());
  }
  return std::nullopt; 
}
std::optional<std::string> getStrAttr(ObjectAttrHandle& attrHandle, const char* key){
  std::optional<std::string*> value = getTypeFromAttr<std::string>(getObjectAttributePtr(attrHandle.id, key));
  if (value.has_value()){
   return *(value.value());
  }
  return std::nullopt; 
}
std::optional<float> getFloatAttr(ObjectAttrHandle& attrHandle, const char* key){
  std::optional<float*> value = getTypeFromAttr<float>(getObjectAttributePtr(attrHandle.id, key));
  if (value.has_value()){
   return *(value.value());
  }
  return std::nullopt; 
}
std::optional<bool> getBoolAttr(ObjectAttrHandle& attrHandle, const char* key){
  std::optional<bool*> value = getTypeFromAttr<bool>(getObjectAttributePtr(attrHandle.id, key));
  if (value.has_value()){
   return *(value.value());
  }
  return std::nullopt; 
}

std::optional<AttributeValue> getAttr(ObjectAttrHandle& attrHandle, const char* key){
  auto attrValue = getObjectAttribute(attrHandle.id, key);
  return attrValue;
}
bool hasAttribute(objid id, const char* key){
  return getObjectAttributePtr(id, key).has_value();
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
  auto objattr = getAttrHandle(id);
  auto attrValue = getFloatAttr(objattr, key);
  return attrValue;
}

void setGameObjectTexture(objid id, std::string texture){
  gameapi -> setSingleGameObjectAttr(id, "texture", texture);
}
void setGameObjectTextureOffset(objid id, glm::vec2 offset){
  gameapi -> setSingleGameObjectAttr(id, "textureoffset", offset);
}
void setGameObjectFriction(objid id, float friction){
  gameapi -> setSingleGameObjectAttr(id, "physics_friction", friction);
}
void setGameObjectVelocity(objid id, glm::vec3 velocity){
  gameapi -> setSingleGameObjectAttr(id, "physics_velocity", velocity);
}
void setGameObjectTint(objid id, glm::vec4 tint){
  gameapi -> setSingleGameObjectAttr(id, "tint", tint);
}
void setGameObjectStateEnabled(objid id, bool enable){
  gameapi -> setSingleGameObjectAttr(id, "state", enable ? std::string("enabled") : std::string("disabled"));
}
void setGameObjectPhysicsDynamic(objid id){
  gameapi -> setSingleGameObjectAttr(id, "physics_type", "dynamic");
}

void setGameObjectPhysics(objid id, float mass, float restitution, float friction, glm::vec3 gravity){
  gameapi -> setGameObjectAttr(
    id, 
    {
      GameobjAttribute { .field = "physics_mass", .attributeValue = mass },
      GameobjAttribute { .field = "physics_restitution", .attributeValue = restitution },
      GameobjAttribute { .field = "physics_friction", .attributeValue = friction },
      GameobjAttribute { .field = "physics_gravity", .attributeValue = gravity },
    }
  );
}
void setGameObjectPhysicsOptions(objid id, glm::vec3 avelocity, glm::vec3 velocity, glm::vec3 angle, glm::vec3 linear, glm::vec3 gravity){
  gameapi -> setGameObjectAttr(
    id, 
    {
      GameobjAttribute { .field = "physics_avelocity", .attributeValue = avelocity },
      GameobjAttribute { .field = "physics_velocity", .attributeValue = velocity },
      GameobjAttribute { .field = "physics_angle", .attributeValue = angle },
      GameobjAttribute { .field = "physics_linear", .attributeValue = linear },
      GameobjAttribute { .field = "physics_gravity", .attributeValue = gravity },
    }
  );
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

std::optional<objid> findObjByShortName(std::string name){
  auto allSceneIds = gameapi -> listScenes(std::nullopt);
  for (auto id : allSceneIds){
    auto objId = gameapi -> getGameObjectByName(name, id, true);
    if (objId.has_value()){
      return objId.value();
    }
  }
  return std::nullopt;
}

std::optional<objid> activeSceneForSelected(){
  auto selected = gameapi -> selected();
  if (selected.size() == 0){
    return std::nullopt;
  }
  auto selectedId = gameapi -> selected().at(0);
  auto sceneId = gameapi -> listSceneId(selectedId);
  return sceneId;
}

void selectWithBorder(glm::vec2 fromPoint, glm::vec2 toPoint){
  float leftX = fromPoint.x < toPoint.x ? fromPoint.x : toPoint.x;
  float rightX = fromPoint.x > toPoint.x ? fromPoint.x : toPoint.x;

  float topY = fromPoint.y < toPoint.y ? fromPoint.y : toPoint.y;
  float bottomY = fromPoint.y > toPoint.y ? fromPoint.y : toPoint.y;

  float width = rightX - leftX;;
  float height = bottomY - topY;

  //std::cout << "selection: leftX = " << leftX << ", rightX = " << rightX << ", topY = " << topY << ", bottomY = " << bottomY << ", width = " << width << ", height = " << height << std::endl;
  float borderSize = 0.005f;
  float borderWidth = width - borderSize;
  float borderHeight = height - borderSize;

  gameapi -> drawRect(leftX + (width * 0.5f), topY + (height * 0.5f), width, height, false, glm::vec4(0.9f, 0.9f, 0.9f, 0.1f), std::nullopt, true, std::nullopt, std::nullopt);
  gameapi -> drawRect(leftX + (width * 0.5f), topY + (height * 0.5f), borderWidth, borderHeight, false, glm::vec4(0.1f, 0.1f, 0.1f, 0.1f), std::nullopt, true, std::nullopt, std::nullopt);

  // this can be amortized over multiple 
  float uvWidth = toPoint.x - fromPoint.x;
  float uvHeight = toPoint.y - fromPoint.y;


  std::set<objid> ids;

  /*for (int x = 0; x < 50; x++){
    for (int y = 0; y < 50; y++){    
      gameapi -> idAtCoordAsync(fromPoint.x + (x * uvWidth / 50.f), fromPoint.y + (y * uvHeight / 50.f), true, [&gameState](std::optional<objid> selectedId, glm::vec2 texCoordUv) -> void {
        if (!selectedId.has_value()){
          return;
        }
        auto selectableValue = getSingleAttr(selectedId.value(), "dragselect");
        if (selectableValue.has_value() && selectableValue.value() == gameState.dragSelect.value()){
          ids.insert(idAtCoord.value());
        }
      });
    } 
  }*/

  //modlog("dragselect", print(ids));
  //gameapi -> setSelected(ids);
}

float randomNum(){
  return static_cast<float>(rand()) / (static_cast<float>(RAND_MAX));
}

void drawCenteredText(std::string text, float ndiOffsetX, float ndiOffsetY, float ndiSize, std::optional<glm::vec4> tint, std::optional<objid> selectionId){
  float fontSizeNdiEquivalent = ndiSize * 1000.f / 2.f;   // 1000 = 1 ndi
  float approximateWidth = text.size() * ndiSize;
  gameapi -> drawText(text, ndiOffsetX - (approximateWidth * 0.5f), ndiOffsetY, fontSizeNdiEquivalent, false, tint, std::nullopt, true, std::nullopt, selectionId);
}


void drawRightText(std::string text, float ndiOffsetX, float ndiOffsetY, float ndiSize, std::optional<glm::vec4> tint, std::optional<objid> selectionId){
  float fontSizeNdiEquivalent = ndiSize * 1000.f / 2.f;   // 1000 = 1 ndi
  gameapi -> drawText(text, ndiOffsetX, ndiOffsetY, fontSizeNdiEquivalent, false, tint, std::nullopt, true, std::nullopt, std::nullopt);
}