#ifndef MOD_AFTERWORLD_QUERY
#define MOD_AFTERWORLD_QUERY

#include "../../ModEngine/src/cscript/cscript_binding.h"

std::string strFromFirstSqlResult(std::vector<std::vector<std::string>>& sqlResult, int index);
float floatFromFirstSqlResult(std::vector<std::vector<std::string>>& sqlResult, int index);
bool boolFromFirstSqlResult(std::vector<std::vector<std::string>>& sqlResult, int index);
glm::vec3 vec3FromFirstSqlResult(std::vector<std::vector<std::string>>& sqlResult, int xIndex, int yIndex, int zIndex); 


#endif