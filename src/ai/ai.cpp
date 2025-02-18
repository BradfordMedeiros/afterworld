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


bool agentExists(AiData& aiData, objid id){
  for (auto &agent : aiData.agents){
    if (agent.id == id){
      return true;
    }
  }
  return false;
}

std::optional<AgentType> agentTypeStr(std::string agentType){
  if (agentType == "basic"){
    return AGENT_BASIC_AGENT;
  }
  if (agentType == "turret"){
    return AGENT_TURRET;
  }
  if (agentType == "tv"){
    return AGENT_TV;
  }
  modassert(false, "invalid aiAgentType");
  return std::nullopt;
}

std::optional<AiAgent*> getAiAgent(AgentType agentType){
  if (agentType == AGENT_BASIC_AGENT){
    return &basicAgent;
  }
  if (agentType == AGENT_TURRET){
    return &turretAgent;
  }
  //if (agentType == AGENT_TV){
  //  return &tvAgent;
  //}
  modassert(false, "invalid aiAgentType");
  return std::nullopt;
}


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
    getAiAgent(agent.type).value() -> detect(worldInfo, agent);
  }
}

void addAiAgent(AiData& aiData, objid id, std::string agentType){
  modassert(!agentExists(aiData, id), std::string("agent already exists: ") + std::to_string(id));
  auto type = agentTypeStr(agentType).value();
  aiData.agents.push_back(Agent {
    .id = id,
    .enabled = true,
    .type = type,
    .agentData = getAiAgent(type).value() -> createAgent(id),
  });
}

void maybeRemoveAiAgent(AiData& aiData, objid id){
  std::vector<Agent> newAgents;
  for (auto &agent : aiData.agents){
    if(agent.id != id){
      newAgents.push_back(agent);
    }
  }
  aiData.agents = newAgents;

  freeState(aiData.worldInfo, id);
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

void onAiHealthChange(objid targetId, float remainingHealth){
  HealthChangeMessage healthMessage {
    .targetId = targetId,
    .remainingHealth = remainingHealth,
  };
  gameapi -> sendNotifyMessage("health-change", healthMessage);
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
    auto goals =  getAiAgent(agent.type).value() -> getGoals(aiData.worldInfo, agent);
    auto optimalGoal = getOptimalGoal(goals);
    //modassert(optimalGoal, "no goal for agent");
    if (optimalGoal){
      //modlog("ai goals", nameForSymbol(optimalGoal -> goaltype));
      getAiAgent(agent.type).value() -> doGoal(aiData.worldInfo, *optimalGoal, agent);
    }
  }
}

void onAiOnMessage(AiData& aiData, std::string& key, std::any& value){
  if (key == "health-change"){
    auto healthChangeMessage = anycast<HealthChangeMessage>(value);
    modassert(healthChangeMessage != NULL, "healthChangeMessage not an healthChangeMessage");

    for (auto &agent : aiData.agents){
      getAiAgent(agent.type).value() -> onHealthChange(agent, healthChangeMessage -> targetId, healthChangeMessage -> remainingHealth);
    }    
  }
}

DebugConfig debugPrintAi(AiData& aiData){
  DebugConfig debugConfig { .data = {} };
  debugConfig.data.push_back({{ "agents", std::to_string(aiData.agents.size()) }});
  return debugConfig;
}

Agent& getAgent(AiData& aiData, objid id){
  for (auto &agent : aiData.agents){
    if (agent.id == id){
      return agent;
    }
  }
  modassert(false, "agent does not exist");
}