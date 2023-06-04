#ifndef MOD_AFTERWORLD_QUERY
#define MOD_AFTERWORLD_QUERY

#include "../../ModEngine/src/cscript/cscript_binding.h"
#include "../../ModEngine/src/translations.h"
#include <bits/stdc++.h>

std::string strFromFirstSqlResult(std::vector<std::vector<std::string>>& sqlResult, int index);
std::string strFromSqlRow(std::vector<std::string>& sqlResult, int index);
float floatFromFirstSqlResult(std::vector<std::vector<std::string>>& sqlResult, int index);
int intFromFirstSqlResult(std::vector<std::vector<std::string>>& sqlResult, int index);
bool boolFromFirstSqlResult(std::vector<std::vector<std::string>>& sqlResult, int index);
glm::vec3 vec3FromFirstSqlResult(std::vector<std::vector<std::string>>& sqlResult, int xIndex, int yIndex, int zIndex); 
glm::vec3 vec3FromFirstSqlResult(std::vector<std::vector<std::string>>& sqlResult, int index); 
glm::vec4 vec4FromFirstSqlResult(std::vector<std::vector<std::string>>& sqlResult, int index);
glm::vec3 vec3FromSqlRow(std::vector<std::string>& sqlRow, int index);
glm::quat quatFromFirstSqlResult(std::vector<std::vector<std::string>>& sqlResult, int index);

std::optional<std::string> getStrAttr(GameobjAttributes& objAttr, std::string key);
std::optional<float> getFloatAttr(GameobjAttributes& objAttr, std::string key);
std::optional<glm::vec3> getVec3Attr(GameobjAttributes& objAttr, std::string key);

float limitAngle(float angleRadians, std::optional<float> minAngle, std::optional<float> maxAngle);

void clickMouse(objid id);

typedef std::function<void(void*, int32_t idAdded, std::string value)> stringAttrFuncValue;
typedef std::function<void(void*, int32_t idAdded, float value)> floatAttrFuncValue;
typedef std::function<void(void*, int32_t idAdded, glm::vec3 value)> vec3AttrFuncValue;

typedef std::variant<stringAttrFuncValue, floatAttrFuncValue, vec3AttrFuncValue> attrFuncValue;
struct AttrFuncValue {
	std::string attr;
	attrFuncValue fn;
};

std::function<void(int32_t, void*, int32_t)> getOnAttrAdds(std::vector<AttrFuncValue> attrFuncs);
struct AttrFunc {
	std::string attr;
	std::function<void(void*, int32_t idAdded)> fn;
};
bool hasAttribute(GameobjAttributes& attributes, std::string attr);
std::function<void(int32_t, void*, int32_t)> getOnAttrRemoved(std::vector<AttrFunc> attrFuncs);

float randomNumber(float min, float max);

int closestHitpoint(std::vector<HitObject>& hitpoints, glm::vec3 playerPos);
void showDebugHitmark(HitObject& hitpoint, objid playerId);

void debugAssertForNow(bool valid, const char* message);

template <typename T>
T* anycast(std::any& anyValue){
  try {
    T* value = std::any_cast<T>(&anyValue);
    modassert(value, "anycast value was NULL");
    return value;
  }catch(...){
    return NULL;
  }
}

std::function<void(int32_t, void*, std::string&, std::any&)> attributeFn(std::function<void(int32_t, void*, std::string&, AttributeValue& value)> fn);


struct DamageMessage {
  objid id;
  float amount;
};

#endif