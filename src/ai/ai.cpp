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


struct AiData {
  WorldInfo worldInfo;
  std::vector<Agent> agents;
};


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
    updateVec3State(worldInfo, getSymbol(stateName) /* this is a leak */, gameapi -> getGameObjectPos(targetId, true), symbols, TargetData { .id = targetId });
  }
}

void detectWorldInfo(WorldInfo& worldInfo, std::vector<Agent>& agents){
  updateWorldStateTargets(worldInfo);
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
    if(gameapi -> gameobjExists(agent.id)){
      newAgents.push_back(agent);
    }
  }
  aiData.agents = newAgents;
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

void doGoal(Goal& goal, Agent& agent){
  if (agent.type == AGENT_BASIC_AGENT){
    doGoalBasicAgent(goal, agent);
    return;
  }else if (agent.type == AGENT_TURRET){
    doGoalTurretAgent(goal, agent);
    return;
  }
  modassert(false, "do goal invalid agent");
}



void onAiFrame(AiData& aiData){
  // probably don't want to reset world state every frame, but ok for now
  // consider what data should and shouldn't be per frame (per data refresh?)
  aiData.worldInfo = WorldInfo { .boolValues = {}, .vec3Values = {} };
  detectWorldInfo(aiData.worldInfo, aiData.agents);

  for (auto &agent : aiData.agents){
    if (!gameapi -> gameobjExists(agent.id)){ 
      continue;
    }
    auto goals = getGoalsForAgent(aiData.worldInfo, agent);
    auto optimalGoal = getOptimalGoal(goals);
    //modassert(optimalGoal, "no goal for agent");
    if (optimalGoal){
      //modlog("ai goals", nameForSymbol(optimalGoal -> goaltype));
      doGoal(*optimalGoal, agent);  
    }
  }
}


CScriptBinding aiBinding(CustomApiBindings& api, const char* name){
  auto binding = createCScriptBinding(name, api);
  
  binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    AiData* aiData = new AiData;
    aiData -> worldInfo = WorldInfo {
      .boolValues = {},
      .vec3Values = {},
    };
    aiData -> agents = {};
    for (auto &agentId : gameapi -> getObjectsByAttr("agent", std::nullopt, std::nullopt)){
      maybeAddAgent(*aiData, agentId);
    }

    return aiData;
  };

  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
    AiData* aiData = static_cast<AiData*>(data);
    delete aiData;
  };

  binding.onFrame = [](int32_t id, void* data) -> void {
    AiData* aiData = static_cast<AiData*>(data);
    onAiFrame(*aiData);
    gameapi -> drawText("agents: " + std::to_string(aiData -> agents.size()), -0.9, 0.0, 8, false, std::nullopt, std::nullopt, true, std::nullopt, std::nullopt);
  };

  binding.onKeyCallback = [](int32_t id, void* data, int key, int scancode, int action, int mods) -> void {
    AiData* aiData = static_cast<AiData*>(data);
    if (key == 'M' && action == 0) { 
      printWorldInfo(aiData -> worldInfo);
    }
  };

  binding.onObjectAdded = [](int32_t _, void* data, int32_t idAdded) -> void {
    AiData* aiData = static_cast<AiData*>(data);
    maybeAddAgent(*aiData, idAdded);
  };
  binding.onObjectRemoved = [](int32_t _, void* data, int32_t idRemoved) -> void {
    AiData* aiData = static_cast<AiData*>(data);
    maybeRemoveAgent(*aiData, idRemoved);
  };

  binding.onMessage = [](int32_t id, void* data, std::string& key, std::any& value){
    AiData* aiData = static_cast<AiData*>(data);
    for (auto &agent : aiData -> agents){
      if (agent.type == AGENT_BASIC_AGENT){
        onMessageBasicAgent(agent, key, value);
        continue;
      }else if (agent.type == AGENT_TURRET){
        continue;
      }
    }
  };

  return binding;
}