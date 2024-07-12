#include "./ai.h"

extern CustomApiBindings* gameapi;

/*
  
  so to open a door 
    - possible goals: open door, unlock door
    - unlock door prereq to open door, so unlock door, then open door

    - kill player
    - take cover prereq to kill player
    - or just kill player directly
    - but those are different nodes and then have different probabilities
    - so will take cover since it'll increase probability


  needs a detectioun system to find possible goals 

  and then going to goals require actions

  actions 
  - move to place, 
  - shoot
  etc
*/





// based on goal-info:targets update vec3 position for each - target-pos-<objid> with team symbol from attr
void updateWorldStateTargets(WorldInfo& worldInfo){
  static int targetSymbol = getSymbol("target");
  auto targetIds = gameapi -> getObjectsByAttr("goal-info", "target", std::nullopt);
  for (auto targetId : targetIds){
    std::string stateName = std::string("target-pos-") + std::to_string(targetId);
    auto team = getSingleAttr(targetId, "team");
    std::set<int> symbols = { targetSymbol };
    if (team.has_value()){
      symbols.insert(getSymbol(team.value()));
    }
    auto position = gameapi -> getGameObjectPos(targetId, true);
    updateState(worldInfo, getSymbol(stateName), EntityPosition { .id = targetId, .position = position }, symbols, STATE_ENTITY_POSITION, 0);
  }
  // agoal-info
}

void updateAmmoLocations(WorldInfo& worldInfo){
  static int ammoSymbol = getSymbol("ammo");
  auto targetIds = gameapi -> getObjectsByAttr("pickup-trigger", "ammo", std::nullopt);
  for (auto targetId : targetIds){
    std::string stateName = std::string("ammo-pos-") + std::to_string(targetId); // leak
    auto position = gameapi -> getGameObjectPos(targetId, true);
    updateState(worldInfo, getSymbol(stateName), EntityPosition { .id = targetId, .position = position }, { ammoSymbol }, STATE_ENTITY_POSITION, targetId);
  }
}

void updatePointsOfInterest(WorldInfo& worldInfo){
  //static int ammoSymbol = getSymbol("ammo");
  auto targetIds = gameapi -> getObjectsByAttr("interest", std::nullopt, std::nullopt);
  for (auto targetId : targetIds){
    std::string stateName = std::string("interest-") + std::to_string(targetId); // leak
    auto attrValue = getSingleAttr(targetId, "interest").value();
    auto tags = getSymbol(std::string("interest-") + attrValue);
    auto position = gameapi -> getGameObjectPos(targetId, true);
    updateState(worldInfo, getSymbol(stateName), EntityPosition { .id = targetId, .position = position }, { tags }, STATE_ENTITY_POSITION, targetId);
  }
}

void detectWorldInfo(WorldInfo& worldInfo, std::vector<Agent>& agents){
  updateWorldStateTargets(worldInfo);
  updateAmmoLocations(worldInfo);
  updatePointsOfInterest(worldInfo);

  for (auto agent : agents){
    if (!gameapi -> gameobjExists(agent.id)){ 
      continue;
    }
    if (agent.type == AGENT_BASIC_AGENT){
      detectWorldInfoBasicAgent(worldInfo, agent);
      continue;
    }else if(agent.type == AGENT_TURRET){
      detectWorldInfoTurretAgent(worldInfo, agent);
      continue;
    }
    modassert(false, "detect world info invalid agent type");
  }
}

bool agentExists(AiData& aiData, objid id){
  for (auto &agent : aiData.agents){
    if (agent.id == id){
      return true;
    }
  }
  return false;
}
void maybeAddAgent(AiData& aiData, objid id){
  if (agentExists(aiData, id)){
    return;
  }
  auto agent = getSingleAttr(id, "agent");
  if (agent.has_value()){
    auto agentType = agent.value();
    if (agentType == "basic"){
      aiData.agents.push_back(createBasicAgent(id));
    }else if (agentType == "turret"){
      aiData.agents.push_back(createTurretAgent(id));
    }else{
      modassert(false, std::string("invalid agent type: ") + agentType);
    }
  }
}

void maybeRemoveAgent(AiData& aiData, objid id){
  std::vector<Agent> newAgents;
  for (auto &agent : aiData.agents){
    if(agent.id != id){
      newAgents.push_back(agent);
    }
  }
  aiData.agents = newAgents;
}


void maybeDisableAi(AiData& aiData, objid id){
  //modassert(false, std::string("disable ai placeholder: ") + gameapi -> getGameObjNameForId(id).value());
  for (auto &agent : aiData.agents){
    if (agent.id == id){
      agent.enabled = false;
      modlog("ai - disable - ", gameapi -> getGameObjNameForId(id).value());
      break;
    }
  }
}
void maybeReEnableAi(AiData& aiData, objid id){
  //modassert(false, std::string("enable ai placeholder: ") + gameapi -> getGameObjNameForId(id).value());
  for (auto &agent : aiData.agents){
    if (agent.id == id){
      agent.enabled = true;
      modlog("ai - enable - ", gameapi -> getGameObjNameForId(id).value());
      break;
    }
  }
}

std::vector<Goal> getGoalsForAgent(WorldInfo& worldInfo, Agent& agent){
  if (agent.type == AGENT_BASIC_AGENT){
    return getGoalsForBasicAgent(worldInfo, agent);
  }else if (agent.type == AGENT_TURRET){
    return getGoalsForTurretAgent(worldInfo, agent);
  }
  modassert(false, "get goals for agent invalid agent type");
  return {};
}

Goal* getOptimalGoal(std::vector<Goal>& goals){
  if (goals.size() == 0){
    return NULL;
  }
  Goal& goal = goals.at(0);
  int maxScore = goal.score(goal.goalData);
  int maxScoreIndex = 0;
  for (int i = 1; i < goals.size(); i++){
    Goal& goal = goals.at(i);
    auto goalScore = goal.score(goal.goalData);
    if (goalScore > maxScore){
      maxScore = goalScore;
      maxScoreIndex = i;
    }
  }
  return &goals.at(maxScoreIndex);
}

void doGoal(WorldInfo& worldInfo, Goal& goal, Agent& agent){
  if (agent.type == AGENT_BASIC_AGENT){
    doGoalBasicAgent(worldInfo, goal, agent);
    return;
  }else if (agent.type == AGENT_TURRET){
    doGoalTurretAgent(worldInfo, goal, agent);
    return;
  }
  modassert(false, "do goal invalid agent");
}

void onAiHealthChange(objid targetId, float remainingHealth){
  HealthChangeMessage healthMessage {
    .targetId = targetId,
    .remainingHealth = remainingHealth,
  };
  gameapi -> sendNotifyMessage("health-change", healthMessage);
}

void onMessageBasicAgent(Agent& agent, std::string& key, std::any& value){
  if(key == "ammo"){
    auto itemAcquiredMessage = anycast<ItemAcquiredMessage>(value);
    modassert(itemAcquiredMessage != NULL, "ammo message not an ItemAcquiredMessage");
    onAiAmmo(agent, itemAcquiredMessage -> targetId, itemAcquiredMessage -> amount);
  }else if (key == "health-change"){
    auto healthChangeMessage = anycast<HealthChangeMessage>(value);
    modassert(healthChangeMessage != NULL, "healthChangeMessage not an healthChangeMessage");
    onAiHealthChange(agent, healthChangeMessage -> targetId, healthChangeMessage -> remainingHealth);
  }
}


AiData createAiData(){
  AiData aiData;
  aiData.worldInfo = WorldInfo {
    .anyValues = {},
  };
  aiData.agents = {};
  return aiData;
}

// probably most of this doesn't need to run on every frame except probably doGoal
// (which could probably not run every frame too if everything in it is only a state transition
//   as opposed to eg actually doing the movement)
void onFrameAi(AiData& aiData){
  if (isPaused()){
    return;
  }
  
  detectWorldInfo(aiData.worldInfo, aiData.agents);

  for (auto &agent : aiData.agents){
    if (!agent.enabled){
      continue;
    }
    if (!gameapi -> gameobjExists(agent.id)){ 
      continue;
    }
    auto goals = getGoalsForAgent(aiData.worldInfo, agent);
    auto optimalGoal = getOptimalGoal(goals);
    //modassert(optimalGoal, "no goal for agent");
    if (optimalGoal){
      //modlog("ai goals", nameForSymbol(optimalGoal -> goaltype));
      doGoal(aiData.worldInfo, *optimalGoal, agent);  
    }
  }
}

void onAiObjectAdded(AiData& aiData, int32_t idAdded){
  maybeAddAgent(aiData, idAdded);
}
void onAiObjectRemoved(AiData& aiData, int32_t idRemoved){
  maybeRemoveAgent(aiData, idRemoved);
  freeState(aiData.worldInfo, idRemoved);
}
void onAiOnMessage(AiData& aiData, std::string& key, std::any& value){
  for (auto &agent : aiData.agents){
    if (agent.type == AGENT_BASIC_AGENT){
      onMessageBasicAgent(agent, key, value);
      continue;
    }else if (agent.type == AGENT_TURRET){
      continue;
    }
  }
}

DebugConfig debugPrintAi(AiData& aiData){
  DebugConfig debugConfig { .data = {} };
  debugConfig.data.push_back({{ "agents", std::to_string(aiData.agents.size()) }});
  return debugConfig;
}