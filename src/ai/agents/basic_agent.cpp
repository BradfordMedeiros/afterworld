#include "./basic_agent.h"

extern CustomApiBindings* gameapi;

struct AgentAttackState {
  float lastAttackTime;
  std::optional<GunCore> gunCore;
};

Agent createBasicAgent(objid id){
	return Agent{
    .id = id,
    .type = AGENT_BASIC_AGENT,
    .agentData = AgentAttackState {
      .lastAttackTime = 0.f,
      .gunCore = createGunCoreInstance("pistol", 5, gameapi -> listSceneId(id)),
    },
  };
}

void detectWorldInfoBasicAgent(WorldInfo& worldInfo, Agent& agent){
  auto visibleTargets = checkVisibleTargets(worldInfo, agent.id);
  if (visibleTargets.size() > 0){
    updateVec3State(worldInfo, getSymbol(std::string("agent-can-see-pos-agent") + std::to_string(agent.id) /* bad basically a small leak */ ), visibleTargets.at(0).position);

    auto symbol = getSymbol(std::string("agent-can-see-pos-agent") + std::to_string(agent.id) /* bad basically a small leak */ ); 
    updateState(
      worldInfo, 
      symbol, 
      visibleTargets.at(0).position, 
      {}, 
      STATE_VEC3
    );
  }
}

std::vector<Goal> getGoalsForBasicAgent(WorldInfo& worldInfo, Agent& agent){
  static int idleGoal = getSymbol("idle");
  static int moveToTargetGoal = getSymbol("move-to-target");
  static int attackTargetGoal = getSymbol("attack-target");

  std::vector<Goal> goals = {};

  goals.push_back(
    Goal {
      .goaltype = idleGoal,
      .goalData = NULL,
      .score = [&agent](std::any& targetPosition) -> int { 
        return 5;
      }
    }
  );


  auto symbol = getSymbol(std::string("agent-can-see-pos-agent") + std::to_string(agent.id));
  auto targetPosition = getState<glm::vec3>(worldInfo, symbol);

  if (targetPosition.has_value()){
    goals.push_back(
      Goal {
        .goaltype = moveToTargetGoal,
        .goalData = targetPosition.value(),
        .score = [&agent](std::any& targetPosition) -> int { 
          return 10;
        }
      }
    );
  }


  goals.push_back(
    Goal {
      .goaltype = attackTargetGoal,
      .goalData = NULL,
      .score = [&agent, &worldInfo, symbol](std::any&) -> int {
          auto targetPosition = getState<glm::vec3>(worldInfo, symbol);
          if (targetPosition.has_value()){
            auto distance = glm::distance(targetPosition.value(), gameapi -> getGameObjectPos(agent.id, true));
            if (distance < 5){
              return 100;
            }
          }
          return 0;
      }
    }
  );

  return goals;
}


void fireProjectile(objid agentId, AgentAttackState& agentAttackState){
  modlog("basic agent", "firing projectile");
  if (agentAttackState.gunCore.has_value()){
    fireGunAndVisualize(agentAttackState.gunCore.value(), false, true, std::nullopt, agentId);
  }
}

void moveToTarget(objid agentId, glm::vec3 targetPosition){
  // right now this is just setting the obj position, but probably should rely on the navmesh, probably should be applying impulse instead?
  auto agentPos = gameapi -> getGameObjectPos(agentId, true);
  auto towardTarget = gameapi -> orientationFromPos(agentPos, glm::vec3(targetPosition.x, agentPos.y, targetPosition.z));
  auto newPos = gameapi -> moveRelative(agentPos, towardTarget, 5.f * gameapi -> timeElapsed());
  gameapi -> setGameObjectPosition(agentId, newPos, true); 
  gameapi -> setGameObjectRot(agentId, towardTarget, true);
}
void attackTarget(Agent& agent){
  AgentAttackState* attackState = anycast<AgentAttackState>(agent.agentData);
  modassert(attackState, "attackState invalid");

  float currentTime = gameapi -> timeSeconds(false);
  if (currentTime - attackState -> lastAttackTime > 1.f){
    std::cout << "attack placeholder" << std::endl;
    attackState -> lastAttackTime = currentTime;
    fireProjectile(agent.id, *attackState);
  }
}

void doGoalBasicAgent(Goal& goal, Agent& agent){
  static int idleGoal = getSymbol("idle");
  static int moveToTargetGoal = getSymbol("move-to-target");
  static int attackTargetGoal = getSymbol("attack-target");

  if (goal.goaltype == idleGoal){
    // do nothing
    gameapi -> sendNotifyMessage("trigger", AnimationTrigger {
      .entityId = agent.id,
      .transition = "not-walking",
    });
  }else if (goal.goaltype == moveToTargetGoal){
    auto targetPosition = anycast<glm::vec3>(goal.goalData);
    modassert(targetPosition, "target pos was null");
    moveToTarget(agent.id, *targetPosition);
    gameapi -> sendNotifyMessage("trigger", AnimationTrigger {
      .entityId = agent.id,
      .transition = "walking",
    });
  }else if (goal.goaltype == attackTargetGoal){
    // not yet implemented
    attackTarget(agent);
    gameapi -> sendNotifyMessage("trigger", AnimationTrigger {
      .entityId = agent.id,
      .transition = "not-walking",
    });
  }
}

void onMessageBasicAgent(Agent& agent, std::string& key, std::any& value){
  // i prefer ai to just defer to some static ammo management system, but messaging ok for now

  if(key == "ammo"){
    auto itemAcquiredMessage = anycast<ItemAcquiredMessage>(value);
    modassert(itemAcquiredMessage != NULL, "ammo message not an ItemAcquiredMessage");
    if (itemAcquiredMessage -> targetId == agent.id){
      AgentAttackState* attackState = anycast<AgentAttackState>(agent.agentData);
      modassert(attackState, "attackState invalid");
      if (attackState -> gunCore.has_value()){
        deliverAmmo(attackState -> gunCore.value(), itemAcquiredMessage -> amount);
      }
    }
  }
}