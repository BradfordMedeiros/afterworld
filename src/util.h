#ifndef MOD_AFTERWORLD_QUERY
#define MOD_AFTERWORLD_QUERY

#include "../../ModEngine/src/cscript/cscript_binding.h"

std::string strFromFirstSqlResult(std::vector<std::vector<std::string>>& sqlResult, int index);
std::string strFromSqlRow(std::vector<std::string>& sqlResult, int index);
float floatFromFirstSqlResult(std::vector<std::vector<std::string>>& sqlResult, int index);
bool boolFromFirstSqlResult(std::vector<std::vector<std::string>>& sqlResult, int index);
glm::vec3 vec3FromFirstSqlResult(std::vector<std::vector<std::string>>& sqlResult, int xIndex, int yIndex, int zIndex); 
glm::vec3 vec3FromFirstSqlResult(std::vector<std::vector<std::string>>& sqlResult, int index); 
glm::vec3 vec3FromSqlRow(std::vector<std::string>& sqlRow, int index);

std::optional<std::string> getStrAttr(GameobjAttributes& objAttr, std::string key);
std::optional<float> getFloatAttr(GameobjAttributes& objAttr, std::string key);

void debugAssertForNow(bool valid, const char* message);

#endif