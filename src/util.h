#ifndef MOD_AFTERWORLD_QUERY
#define MOD_AFTERWORLD_QUERY

#include "../../ModEngine/src/cscript/cscript_binding.h"
#include "../../ModEngine/src/translations.h"
#include "../../ModEngine/src/common/symbols.h"
#include "../../ModEngine/src/main_api.h"

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
float interpolateDuration(float min, float max, float elapsedTime, float duration);

void clickMouse(objid id);


float randomNumber(float min, float max);
int randomNumber(int min, int max);

std::optional<int> closestHitpoint(std::vector<HitObject>& hitpoints, glm::vec3 playerPos, std::optional<objid> excludeHitpoint);
void drawDebugHitmark(HitObject& hitpoint, objid playerId);

void debugAssertForNow(bool valid, const char* message);

struct HealthChangeMessage {
	objid targetId;
	float remainingHealth;
};

struct ItemAcquiredMessage {
	objid targetId;
	int amount;
};

std::optional<std::string> getSingleAttr(objid id, const char* key);
std::optional<glm::vec3> getSingleVec3Attr(objid id, const char* key);
std::optional<float> getSingleFloatAttr(objid id, const char* key);


struct ObjectAttrHandle { 
  objid id;
};
ObjectAttrHandle getAttrHandle(objid id);
std::optional<glm::vec2> getVec2Attr(ObjectAttrHandle& attrHandle, std::string key);
std::optional<glm::vec3> getVec3Attr(ObjectAttrHandle& attrHandle, std::string key);
std::optional<glm::vec4> getVec4Attr(ObjectAttrHandle& attrHandle, std::string key);
std::optional<std::string> getStrAttr(ObjectAttrHandle& attrHandle, const char* key);
std::optional<float> getFloatAttr(ObjectAttrHandle& attrHandle, const char* key);
std::optional<bool> getBoolAttr(ObjectAttrHandle& attrHandle, const char* key);
std::optional<AttributeValue> getAttr(ObjectAttrHandle& attrHandle, const char* key);
bool hasAttribute(objid id, const char* key);

void setGameObjectTexture(objid id, std::string texture);
void setGameObjectTextureOffset(objid id, glm::vec2 offset);
void setGameObjectFriction(objid id, float friction);
void setGameObjectGravity(objid id, glm::vec3 gravity);
void setGameObjectVelocity(objid id, glm::vec3 velocity);
glm::vec3 getGameObjectVelocity(objid id);
void setGameObjectTint(objid id, glm::vec4 tint);
void setGameObjectStateEnabled(objid id, bool enable);
void setGameObjectMeshEnabled(objid id, bool enable);
void setGameObjectPhysicsDynamic(objid id);
void setGameObjectPhysicsEnable(objid id, bool enable);
void setGameObjectPhysics(objid id, float mass, float restitution, float friction, glm::vec3 gravity);
void setGameObjectPhysicsOptions(objid id, glm::vec3 avelocity, glm::vec3 velocity, glm::vec3 angle, glm::vec3 linear, glm::vec3 gravity);
void setGameObjectEmission(objid id, std::optional<glm::vec3> emission);

void setAmbientLight(glm::vec3 light);

std::string uniqueNameSuffix();

std::optional<std::string> getStrWorldState(const char* object, const char* attribute);
std::optional<AttributeValue> getWorldStateAttr(const char* object, const char* attribute);
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

void playMusicClipById(objid id, std::optional<float> volume, std::optional<glm::vec3> position);
void playGameplayClip(std::string&& clipName, objid sceneId, std::optional<float> volume, std::optional<glm::vec3> position);
void playGameplayClipById(objid id, std::optional<float> volume, std::optional<glm::vec3> position);

std::optional<objid> findObjByShortName(std::string name, std::optional<objid> sceneId);
std::optional<objid> activeSceneForSelected();

void selectWithBorder(glm::vec2 fromPoint, glm::vec2 toPoint);
float randomNum();

void drawCenteredText(std::string text, float ndiOffsetX, float ndiOffsetY, float ndiSize, std::optional<glm::vec4> tint, std::optional<objid> selectionId, std::optional<objid> textureId);

void drawRightText(std::string text, float ndiOffsetX, float ndiOffsetY, float ndiSize, std::optional<glm::vec4> tint, std::optional<objid> selectionId, std::optional<objid> textureId);

struct DebugItem {
	std::string text;
	std::function<void()> onClick;
};
typedef std::variant<std::string, DebugItem> DebugConfigType;
struct DebugConfig {
	std::vector<std::vector<DebugConfigType>> data;
};


#endif