#ifndef MOD_AFTERWORLD_AI_AGENT_COMMON
#define MOD_AFTERWORLD_AI_AGENT_COMMON

#include "../../util.h"
#include "../worldinfo.h"

struct AIInterface {
  std::function<void(objid agentId, glm::vec3 targetPosition, float speed)> move;
  std::function<void(objid agentId)> fireGun;
  std::function<void(objid agentId, int amount)> deliverAmmo;
};

enum AgentType { AGENT_BASIC_AGENT, AGENT_TURRET };
struct Agent { 
  objid id;
  bool enabled;
  AgentType type;
  std::any agentData;
};

struct Goal {
  int goaltype;
  std::any goalData;
  std::function<int(std::any&)> score;
};

struct TargetData {
  objid id;
};

struct IdAndPosition {
  objid id;
  glm::vec3 position;
};

std::vector<IdAndPosition> checkVisibleTargets(WorldInfo& worldInfo, objid agentId);

#endif