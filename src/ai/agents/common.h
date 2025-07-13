#ifndef MOD_AFTERWORLD_AI_AGENT_COMMON
#define MOD_AFTERWORLD_AI_AGENT_COMMON

#include "../../util.h"
#include "../worldinfo.h"
#include "../../resources/paths.h"

struct AIInterface {
  std::function<void(objid agentId, glm::vec3 targetPosition, float speed)> move;
  std::function<void(objid agentId)> stopMoving;
  std::function<void(objid agentId, glm::quat dir)> look;
  std::function<void(objid agentId)> fireGun;
  std::function<void(objid agentId, const char* gun)> changeGun;
  std::function<void(objid agentId, const char* profile)> changeTraits;
  std::function<void(objid agentId, const char* animation, AnimationType animationType)> playAnimation;
  std::function<void(objid agentId, float amount)> doDamage;
};

enum AgentType { AGENT_BASIC_AGENT, AGENT_TURRET, AGENT_TV, AGENT_CRAWLER };
struct Agent { 
  objid id;
  bool enabled;
  AgentType type;
  std::any agentData;

  std::optional<objid> targetId;
  int agentIndex;
  int lastAiDetect;
};


struct Goal {
  int goaltype;
  std::any goalData;
  std::function<int(std::any&)> score;
};

struct AiAgent {
  std::function<std::any(objid)> createAgent;
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

glm::vec3 getAgentPos(WorldInfo& worldInfo, Agent& agent);

std::optional<objid> getAgentTargetId(WorldInfo& worldInfo, Agent& agent);
std::optional<glm::vec3> getAgentTargetPos(WorldInfo& worldInfo, Agent& agent);
void setAgentTargetId(WorldInfo& worldInfo, Agent& agent, objid targetId);

std::vector<EntityPosition> getAmmoPositions(WorldInfo& worldInfo);
std::vector<EntityPosition> getWorldStateTargets(WorldInfo& worldInfo);
std::vector<EntityPosition> getPointsOfInterest(WorldInfo& worldInfo);

#endif