#include "./tv.h"

extern CustomApiBindings* gameapi;
extern AIInterface aiInterface;

std::optional<objid> findBodyPart(objid entityId, const char* part);
void setGameObjectTint(objid id, glm::vec4 tint);
void setGameObjectTexture(objid id, std::string texture);


struct TvAiState {
  std::optional<float> activateTime;
  float lastAttackTime;
  bool changedGun;
  bool usingDefaultTraits;
};

std::any createTvAgent(objid id){
	return TvAiState {
		.activateTime = std::nullopt,
    .lastAttackTime = 0.f,
    .changedGun = false,
    .usingDefaultTraits = false,
	};
}

void detectWorldInfoTvAgent(WorldInfo& worldInfo, Agent& agent){
  auto targetId = getAgentTargetId(worldInfo, agent);
  if (targetId.has_value()){
    return;
  }
  auto visibleTargets = checkVisibleTargets(worldInfo, agent.id);
  if (visibleTargets.size() > 0){
    setAgentTargetId(worldInfo, agent, visibleTargets.at(0).id);
  }
}
std::vector<Goal> getGoalsForTvAgent(WorldInfo& worldInfo, Agent& agent){
  static int idleGoal = getSymbol("idle");
  static int attackTargetGoal = getSymbol("attack-target");

  std::vector<Goal> goals = {};

  TvAiState* tvState = anycast<TvAiState>(agent.agentData);
  modassert(tvState, "attackState invalid");

  auto targetPosition = getAgentTargetPos(worldInfo, agent);

  if (targetPosition.has_value()){
    auto distanceToTarget = glm::distance(targetPosition.value(), gameapi -> getGameObjectPos(agent.id, true, "[gamelogic] getGoalsForTvAgent"));
    if (distanceToTarget < 20 && (tvState -> activateTime.has_value() && (gameapi -> timeSeconds(false) - tvState -> activateTime.value()) > 0.5f)){ // allow the animation to play
      goals.push_back(
        Goal {
          .goaltype = attackTargetGoal,
          .goalData = NULL,
          .score = [&agent](std::any&) -> int { 
            return 10;
          }
        }
      );
    }
  }

  if (!targetPosition.has_value()){
    goals.push_back(
      Goal {
        .goaltype = idleGoal,
        .goalData = NULL,
        .score = [&agent](std::any&) -> int { 
          return 5;
        }
      }
    );   
  }


	return goals;

}

std::optional<float> heightAboveGround(Agent& agent){
  auto position = gameapi -> getGameObjectPos(agent.id, true, "[gamelogic] agents - tv - heightAboveGround");
  auto rotation = gameapi -> orientationFromPos(position, position + glm::vec3(0.f, -10.f, 0.f));
  auto hitpoints = gameapi -> raycast(position, rotation, 100.f);

  std::optional<float> minDistance;
  for (auto &hitpoint : hitpoints){
    if (agent.id == hitpoint.id){
      continue;
    }
    auto distance = glm::distance(hitpoint.point, position);
    if (!minDistance.has_value()){
      minDistance = distance;
    }else if (distance < minDistance.value()){
      minDistance = distance;
    }
  }
  return minDistance;
}

void doGoalTvAgent(WorldInfo& worldInfo, Goal& goal, Agent& agent){
  TvAiState* tvState = anycast<TvAiState>(agent.agentData);

  static int idleGoal = getSymbol("idle");
  static int attackTargetGoal = getSymbol("attack-target");

  if (goal.goaltype == idleGoal){
    if (!tvState -> usingDefaultTraits){
      tvState -> usingDefaultTraits = true;
      aiInterface.changeTraits(agent.id, "default");
    }
    aiInterface.stopMoving(agent.id);

  }else if (goal.goaltype == attackTargetGoal){
    if (tvState -> usingDefaultTraits){
      tvState -> usingDefaultTraits = false;
      aiInterface.changeTraits(agent.id, "tv");
    }

    if (!tvState -> changedGun){
      tvState -> changedGun = true;
      aiInterface.changeGun(agent.id, "pistol");
    }

  	float currentTime = gameapi -> timeSeconds(false);

    auto targetPosition = getAgentTargetPos(worldInfo, agent);
    modassert(targetPosition.has_value(), "doGoalTvAgent does not have a target");

  	auto agentPos = getAgentPos(worldInfo, agent);;

    auto groundHeight = heightAboveGround(agent);
    auto amountToMoveY = !groundHeight.has_value() ? 0.f : (1.f - groundHeight.value());

  	glm::vec3 targetPosSameY = glm::vec3(targetPosition.value().x, agentPos.y , targetPosition.value().z);
    auto towardTarget = gameapi -> orientationFromPos(agentPos, targetPosSameY);
    auto newPosition = glm::vec3(targetPosition.value().x, targetPosition.value().y + amountToMoveY, targetPosition.value().z);
    
    //aiInterface.look(agent.id, towardTarget);
    aiInterface.move(agent.id, newPosition,  1.f);

    auto currTime = gameapi -> timeSeconds(false);
    if (true || (currTime - tvState -> lastAttackTime) > 0.5f){
      aiInterface.look(agent.id, towardTarget);
      aiInterface.fireGun(agent.id);
      tvState -> lastAttackTime = currTime;
      modlog("tv", "attack");
    }


    modlog("tv groundHeight", std::to_string(groundHeight.value()));
    modlog("tv agentPos", print(agentPos));
    modlog("tv newPosition", print(newPosition));

  }
}
void onAiTvAgentHealthChange(Agent& agent, objid targetId, float remainingHealth){
  if (agent.id == targetId){
    setTvActive(agent, true);
  }
}

void setTvActive(Agent& agent, bool active){
	modlog("ai tv", "setTvActive");
	TvAiState* tvState = anycast<TvAiState>(agent.agentData);
  bool oldIsActive = tvState -> activateTime.has_value();

  if (active && oldIsActive){
    return;
  }
  if (!active && !oldIsActive){
    return;
  }

  tvState -> activateTime = active ? gameapi -> timeSeconds(false) : std::optional<float>(std::nullopt);

  auto screen = findBodyPart(agent.id, "screen");;
  if (active){
    setGameObjectTint(screen.value(), glm::vec4(1.f, 0.f, 0.f, 1.f));
    setGameObjectTexture(screen.value(), paths::AI_TV_HAPPY_FACE);
  }else{
    setGameObjectTint(screen.value(), glm::vec4(1.f, 1.f, 1.f, 1.f));
  }

  if (oldIsActive != tvState -> activateTime.has_value()){
    if (active){
      modlog("ai tv active active", std::to_string(agent.id));
    }else{
      modlog("ai tv active inactive", std::to_string(agent.id));
    }
    aiInterface.playAnimation(agent.id, active ? "activate" : "deactivate", FORWARDS);
  }
  if (!tvState -> activateTime.has_value()){
    aiInterface.stopMoving(agent.id);
  }

}

bool isAgentTv(Agent& agent){
  TvAiState* tvState = anycast<TvAiState>(agent.agentData);
  return tvState != NULL;
}

AiAgent tvAgent{
  .createAgent = createTvAgent,
  .detect = detectWorldInfoTvAgent,
  .getGoals = getGoalsForTvAgent,
  .doGoal = doGoalTvAgent,
  .onHealthChange = onAiTvAgentHealthChange,
};