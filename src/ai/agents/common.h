#ifndef MOD_AFTERWORLD_AI_AGENT_COMMON
#define MOD_AFTERWORLD_AI_AGENT_COMMON

#include "../../util.h"
#include "../worldinfo.h"

struct AIInterface {
  std::function<void(objid agentId, glm::vec3 targetPosition, float speed)> move;
  std::function<void(objid agentId, glm::quat dir)> look;
  std::function<void(objid agentId)> fireGun;
  std::function<void(objid agentId, const char* gun)> changeGun;
  std::function<void(objid agentId, const char* animation, AnimationType animationType)> playAnimation;
};

enum AgentType { AGENT_BASIC_AGENT, AGENT_TURRET, AGENT_TV };
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

struct AiAgent {
  std::function<Agent(objid)> createAgent;
  std::function<void(WorldInfo&, Agent&)> detect;
  std::function<std::vector<Goal>(WorldInfo&, Agent&)> getGoals;
  std::function<void(WorldInfo&, Goal&, Agent&)> doGoal;

  std::function<void(Agent& agent, objid targetId, float remainingHealth)> onHealthChange;
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