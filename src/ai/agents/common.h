#ifndef MOD_AFTERWORLD_AI_AGENT_COMMON
#define MOD_AFTERWORLD_AI_AGENT_COMMON

#include "../../util.h"
#include "../worldinfo.h"

enum AgentType { AGENT_BASIC_AGENT, AGENT_TURRET };
struct Agent { 
  objid id;
  AgentType type;
};

struct Goal {
  int goaltype;
  std::any goalData;
  std::function<int(std::any&)> score;
};

#endif