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
};

void detectWorldInfo(WorldInfo& worldInfo, std::vector<Agent>& agents){
  static int targetSymbol = getSymbol("target");

  auto targetIds = gameapi -> getObjectsByAttr("goal-info", "target", std::nullopt);
  for (auto targetId : targetIds){
    std::string stateName = std::string("target-pos-") + std::to_string(targetId);
    auto team = getSingleAttr(targetId, "team");
    std::set<int> symbols = { targetSymbol };
    if (team.has_value()){
      symbols.insert(getSymbol(team.value()));
    }
    updateVec3State(worldInfo, getSymbol(stateName), gameapi -> getGameObjectPos(targetId, true), symbols);
  }
  modassert(targetIds.size() >= 1, "need >= 1 target");
  updateVec3State(worldInfo, getSymbol("target-position"), gameapi -> getGameObjectPos(targetIds.at(0), true));

  for (auto agent : agents){
    if (agent.type == AGENT_BASIC_AGENT){
      detectWorldInfoBasicAgent(worldInfo, agent);
      continue;
    }
    modassert(false, "detect world info invalid agent type");
  }
}



std::vector<Agent> createAgents(){
  auto agentIds = gameapi -> getObjectsByAttr("agent", std::nullopt, std::nullopt);
  std::vector<Agent> agents;
  for (auto &id : agentIds){
    agents.push_back(createBasicAgent(id));
  }

  return agents;
}


std::vector<Goal> getGoalsForAgent(WorldInfo& worldInfo, Agent& agent){
  if (agent.type == AGENT_BASIC_AGENT){
    return getGoalsForBasicAgent(worldInfo, agent);
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
  }
  modassert(false, "do goal invalid agent");
}


CScriptBinding aiBinding(CustomApiBindings& api, const char* name){
  auto binding = createCScriptBinding(name, api);
  
  binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    AiData* aiData = new AiData;
    aiData -> worldInfo = WorldInfo {
      .boolValues = {},
      .vec3Values = {},
    };
    return aiData;
  };

  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
    AiData* aiData = static_cast<AiData*>(data);
    delete aiData;
  };

  binding.onFrame = [](int32_t id, void* data) -> void {
    AiData* aiData = static_cast<AiData*>(data);
    auto agents = createAgents();

    // probably don't want to reset world state every frame, but ok for now
    // consider what data should and shouldn't be per frame (per data refresh?)
    aiData -> worldInfo = WorldInfo { .boolValues = {}, .vec3Values = {} };
    detectWorldInfo(aiData -> worldInfo, agents);

    for (auto &agent : agents){
      auto goals = getGoalsForAgent(aiData -> worldInfo, agent);
      auto optimalGoal = getOptimalGoal(goals);
      //modassert(optimalGoal, "no goal for agent");
      if (optimalGoal){
        doGoal(*optimalGoal, agent);  
      }
    }
  };

  binding.onKeyCallback = [](int32_t id, void* data, int key, int scancode, int action, int mods) -> void {
    AiData* aiData = static_cast<AiData*>(data);
    if (key == 'M' && action == 0) { 
      printWorldInfo(aiData -> worldInfo);
    }else if (key == ',' && action == 0){
      auto spawnpointIds  = gameapi -> getObjectsByAttr("spawn", std::nullopt, std::nullopt);
      for (auto spawnpointId : spawnpointIds){
        spawnPlayer(spawnpointId, "red");
      }
    }else if (key == '.' && action == 0){
      auto spawnpointIds  = gameapi -> getObjectsByAttr("spawn", std::nullopt, std::nullopt);
      for (auto spawnpointId : spawnpointIds){
        spawnPlayer(spawnpointId, "blue");
      }
    }else if (key == '/' && action == 0){
      auto spawnpointIds  = gameapi -> getObjectsByAttr("spawn-managed", std::nullopt, std::nullopt);
      for (auto spawnpointId : spawnpointIds){
        gameapi -> removeObjectById(spawnpointId);
      }
    }
  };


  return binding;
}