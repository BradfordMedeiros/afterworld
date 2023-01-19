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

std::optional<std::string> getStrAttr(GameobjAttributes& objAttr, std::string key){
  if (objAttr.stringAttributes.find(key) != objAttr.stringAttributes.end()){
    return objAttr.stringAttributes.at(key);
  }
  return std::nullopt;
}

std::optional<float> getFloatAttr(GameobjAttributes& objAttr, std::string key){
  if (objAttr.numAttributes.find(key) != objAttr.numAttributes.end()){
    return objAttr.numAttributes.at(key);
  }
  return std::nullopt;
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

bool assertDebug = false;
void debugAssertForNow(bool valid, const char* message){
  if (assertDebug){
    modassert(valid, message);
  }
}

