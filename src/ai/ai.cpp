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
  if (agentType == "crawler"){
    return AGENT_CRAWLER;
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
  if (agentType == AGENT_TV){
    return &tvAgent;
  }
  if (agentType == AGENT_CRAWLER){
    return &crawlerAgent;
  }
  modassert(false, "invalid aiAgentType");
  return std::nullopt;
}


void onObjAdded(AiData& aiData, objid id){
  {
    static int targetSymbol = getSymbol("target");
    auto goalInfo = getSingleAttr(id, "goal-info");
    if(goalInfo.has_value()){
      if (goalInfo.has_value() && goalInfo.value() == "target"){
        auto targetId = id;
        std::string stateName = std::string("target-pos-") + std::to_string(targetId);
        auto team = getSingleAttr(targetId, "team");
        std::set<int> symbols = { targetSymbol };
        if (team.has_value()){
          symbols.insert(getSymbol(team.value()));
        }
        auto position = gameapi -> getGameObjectPos(targetId, true, "[gamelogic] ai updateWorldStateTargets");
        modlog("ai obj target added", std::to_string(targetId));
        modlog("ai obj target worldInfo size: ", std::to_string(aiData.worldInfo.anyValues.size()));
        updateState(aiData.worldInfo, getSymbol(stateName), EntityPosition { .id = targetId, .position = position }, symbols, STATE_ENTITY_POSITION, id);  
      }
    }
  }

  {
    static int ammoSymbol = getSymbol("ammo");
    auto pickupTrigger = getSingleAttr(id, "pickup-trigger");
    if (pickupTrigger.has_value() && pickupTrigger.value() == "ammo"){
      auto targetId = id;
      std::string stateName = std::string("ammo-pos-") + std::to_string(targetId); // leak
      auto position = gameapi -> getGameObjectPos(targetId, true, "[gamelogic] ai updateAmmoLocations");
      modlog("ai obj target added", std::to_string(targetId));
      modlog("ai obj target worldInfo size: ", std::to_string(aiData.worldInfo.anyValues.size()));
      updateState(aiData.worldInfo, getSymbol(stateName), EntityPosition { .id = targetId, .position = position }, { ammoSymbol }, STATE_ENTITY_POSITION, targetId);
    }
  }

  {
    auto interest = getSingleAttr(id, "interest");
    if (interest.has_value()){
      auto targetId = id;
      std::string stateName = std::string("interest-") + std::to_string(targetId); // leak
      auto attrValue = getSingleAttr(targetId, "interest").value();
      auto tags = getSymbol(std::string("interest-") + attrValue);
      auto position = gameapi -> getGameObjectPos(targetId, true, "[gamelogic] ai updatePointsOfInterest");
      updateState(aiData.worldInfo, getSymbol(stateName), EntityPosition { .id = targetId, .position = position }, { tags }, STATE_ENTITY_POSITION, targetId);
    }
  }
}



void onObjRemoved(AiData& aiData, objid id){
  modlog("ai target removed", std::to_string(id));
  modlog("ai target worldInfo size: ", std::to_string(aiData.worldInfo.anyValues.size()));
  freeState(aiData.worldInfo, id);
  for (auto &agent : aiData.agents){
    if (agent.targetId.has_value() && agent.targetId.value() == id){
      agent.targetId = std::nullopt;
    }
  }
}

void addAiAgent(AiData& aiData, objid id, std::string agentType){
  modassert(!agentExists(aiData, id), std::string("agent already exists: ") + std::to_string(id));
  auto type = agentTypeStr(agentType).value();
  static int agentIndex = 0;
  aiData.agents.push_back(Agent {
    .id = id,
    .enabled = true,
    .type = type,
    .agentData = getAiAgent(type).value() -> createAgent(id),
    .targetId = std::nullopt,
    .agentIndex = agentIndex,
    .lastAiDetect = 0,
  });
  agentIndex++;
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

int AI_TICK_RATE_MS = 1000;  // once every 1s
float lastTickTime = -1 * AI_TICK_RATE_MS;

bool shouldTickAi(float currTime){
  return true;
  auto timeElapsed = currTime - lastTickTime;
  return (timeElapsed * 1000) > AI_TICK_RATE_MS ;
}

void visualizeAiData(AiData& aiData){
  gameapi -> drawText(std::string("number of agents: ") + std::to_string(aiData.agents.size()), 0.f, 0.f, 10, false, std::nullopt, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);

  auto ammoPositions = getAmmoPositions(aiData.worldInfo);
  for (auto &point : ammoPositions){
    gameapi -> drawLine(point.position, point.position + glm::vec3(0.f, 1.f, 0.f), false, -1, glm::vec4(0.f, 1.f, 0.f, 1.f), std::nullopt, std::nullopt);
  }

  auto pointsOfInterest = getPointsOfInterest(aiData.worldInfo);
  for (auto &point : pointsOfInterest){
    gameapi -> drawLine(point.position, point.position + glm::vec3(0.f, 1.f, 0.f), false, -1, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);
  }

  auto targetPositions = getWorldStateTargets(aiData.worldInfo);
  for (auto &point : targetPositions){
    gameapi -> drawLine(point.position, point.position + glm::vec3(0.f, 1.f, 0.f), false, -1, glm::vec4(0.f, 0.f, 1.f, 1.f), std::nullopt, std::nullopt);
  }
}

void onFrameAi(AiData& aiData, bool showDebug){
  if (showDebug){
    visualizeAiData(aiData);
  }

  if (isPaused()){
    return;
  }
  
  float currTime = gameapi -> timeSeconds(false);
  int timeMs = static_cast<int>(currTime * 1000);
  int updateIntervalMs = 5000;
  int remaining = timeMs % updateIntervalMs;

  int numAiUpdatesPerFrame = 0;
  if (shouldTickAi(currTime)){
    modlog("onFrameAi", "detectWorldInfo");
    lastTickTime = currTime;

    for (Agent& agent : aiData.agents){
      auto targetUpdate = (agent.agentIndex % updateIntervalMs);

      auto timeSinceLastDetect = timeMs - agent.lastAiDetect;
      auto shouldUpdate = timeSinceLastDetect > (targetUpdate + updateIntervalMs);

      //std::cout << "onFrameAi: " << timeMs << ", remaining = " << remaining << ", timeSinceLastDetect = " << timeSinceLastDetect <<  ", targetupdate = " << targetUpdate << ", shouldUpdate = " << (shouldUpdate ? "true" : "false") << std::endl;

      if (shouldUpdate){
        numAiUpdatesPerFrame++;
        //std::cout << "onFrameAi: updating: " << agent.id << std::endl;
        if (!gameapi -> gameobjExists(agent.id)){ 
          continue;
        }
        agent.lastAiDetect = timeMs;
        getAiAgent(agent.type).value() -> detect(aiData.worldInfo, agent);        
      }
    }
  }
  std::cout << "onFrameAi numUpdates: " << numAiUpdatesPerFrame << std::endl;

  for (auto &agent : aiData.agents){
    if (!agent.enabled){
      continue;
    }
    if (!gameapi -> gameobjExists(agent.id)){ 
      continue;
    }
    auto goals = getAiAgent(agent.type).value() -> getGoals(aiData.worldInfo, agent);
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

Agent nullAgent{};
Agent& getAgent(AiData& aiData, objid id){
  for (auto &agent : aiData.agents){
    if (agent.id == id){
      return agent;
    }
  }
  modassert(false, "agent does not exist");
  return nullAgent;
}