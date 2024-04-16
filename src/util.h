#ifndef MOD_AFTERWORLD_QUERY
#define MOD_AFTERWORLD_QUERY

#include "../../ModEngine/src/cscript/cscript_binding.h"
#include "../../ModEngine/src/translations.h"
#include "../../ModEngine/src/common/symbols.h"
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
int randomNumber(int min, int max);

int closestHitpoint(std::vector<HitObject>& hitpoints, glm::vec3 playerPos);
void showDebugHitmark(HitObject& hitpoint, objid playerId);

void debugAssertForNow(bool valid, const char* message);

struct DamageMessage {
  objid id;
  float amount;
};

struct NoHealthMessage {
  objid targetId;
  std::optional<std::string> team;
};

struct HealthChangeMessage {
	objid targetId;
	std::optional<objid> originId;
	float damageAmount;
	float remainingHealth;
};

struct AnimationTrigger {
	objid entityId;
	std::string transition;
};

struct ChangeGunMessage {
	int currentAmmo;
	std::string gun;
};

struct CurrentGunMessage {
	int currentAmmo;
	int totalAmmo;
};

struct SetAmmoMessage {
	int currentAmmo;
	std::string gun;
};
struct ItemAcquiredMessage {
	objid targetId;
	int amount;
};

struct ObjectAttrHandle { 
  GameobjAttributes attr;
};
ObjectAttrHandle getAttrHandle(objid id);
std::optional<glm::vec3> getVec3Attr(ObjectAttrHandle& attrHandle, std::string key);
std::optional<glm::vec4> getVec4Attr(ObjectAttrHandle& attrHandle, std::string key);
std::optional<std::string> getStrAttr(ObjectAttrHandle& attrHandle, const char* key);
std::optional<float> getFloatAttr(ObjectAttrHandle& attrHandle, const char* key);

std::optional<std::string> getSingleAttr(objid id, const char* key);
std::optional<glm::vec3> getSingleVec3Attr(objid id, const char* key);
std::optional<float> getSingleFloatAttr(objid id, const char* key);

std::string uniqueNameSuffix();

std::optional<std::string> getStrWorldState(const char* object, const char* attribute);
bool toggleWorldStateBoolStr(const char* object, const char* attribute, const char* enabled = NULL, const char *disabled = NULL);
std::function<void()> getToggleWorldStateBoolStr(const char* object, const char* attribute);
std::function<void()> getToggleWorldStateBoolStr(const char* object, const char* attribute, const char* enabled, const char *disabled);
std::function<void()> getToggleWorldStateSetStr(const char* object, const char* attribute, const char* value);
std::function<void()> getToggleWorldStateSetFloat(const char* object, const char* attribute, float value);

void notYetImplementedAlert();

void setMusicVolume(float volume);
void setGameplayVolume(float volume);
float getMusicVolume();
float getGameplayVolume();

void playMusicClip(std::string&& clipName, objid sceneId, std::optional<float> volume, std::optional<glm::vec3> position);
void playGameplayClip(std::string&& clipName, objid sceneId, std::optional<float> volume, std::optional<glm::vec3> position);
void playGameplayClipById(objid id, std::optional<float> volume, std::optional<glm::vec3> position);



#endif