#ifndef MOD_AFTERWORLD_QUERY
#define MOD_AFTERWORLD_QUERY

#include "../../ModEngine/src/cscript/cscript_binding.h"

float floatFromFirstSqlResult(std::vector<std::vector<std::string>>& sqlResult, int index);
bool boolFromFirstSqlResult(std::vector<std::vector<std::string>>& sqlResult, int index);


#endif