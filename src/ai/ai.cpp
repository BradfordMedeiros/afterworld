#include "./ai.h"

extern CustomApiBindings* gameapi;

/*


struct BoolValue {
  std::string symbol;
  bool truth;
};
struct WorldInfp {
  std::vector<BoolValue> boolValues;
  std::vector<FloatValue> floatValues;
  std::vector<glm::vec3> vec3Values;
};



addWorldInfo(worldInfo, FloatValue { .symbol = symbol('playerhealth'), .value = 100.f })

struct AgentInfo {
  std::vector<BoolValue*> boolvalues;
  std::vector<FloatValue> floatValues;
};

AgentInfo getAgentInfo(std::vector<symbol> symbols){
  return WorldInfo
}

agentBehavior(AgentInfo -> utility)

objectives {
  
  .active

}


  world sta

  ai:
  core loop:
  - detect goals
  - pick highest utility goal
  - action 
  ---> repeast

  behavior tree: 
    - each agent has a state, and gets triggered into other states based on events
    - eg idle -> attacking
  
  goal 
    - goals exist as things in a scene -> real objects with position, thing you can do 
    - of the detected goals, each goal is evaluated with some utility value fn(actor, distance, goal) => number
    - do the goal that has the highest utility

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


enum AgentType { AGENT_MOVER };
struct Agent { 
  objid id;
  AgentType type;
};

struct AiData {
  WorldInfo worldInfo;
};

void detectWorldInfo(WorldInfo& worldInfo, std::vector<Agent>& agents){
  updateBoolState(worldInfo, "is-night-time", false);
  updateBoolState(worldInfo, "is-day-time", true);

  auto targetIds = gameapi -> getObjectsByAttr("goal-info", "target", std::nullopt);
  for (auto targetId : targetIds){
    std::string stateName = std::string("target-pos-") + std::to_string(targetId);
    auto team = getSingleAttr(targetId, "team");
    std::set<int> symbols = { getSymbol("target") };
    if (team.has_value()){
      symbols.insert(getSymbol(team.value()));
    }
    updateVec3State(worldInfo, stateName, gameapi -> getGameObjectPos(targetId, true), symbols);
  }
  modassert(targetIds.size() >= 1, "need >= 1 target");
  updateVec3State(worldInfo, "target-position", gameapi -> getGameObjectPos(targetIds.at(0), true));

  auto agentIds = gameapi -> getObjectsByAttr("agent", std::nullopt, std::nullopt);
  for (auto agentId : agentIds){
    std::string stateName = std::string("agent-pos-") + std::to_string(agentId);
    updateVec3State(worldInfo, stateName, gameapi -> getGameObjectPos(agentId, true));
  }
}



std::vector<Agent> createAgents(){
  auto agentIds = gameapi -> getObjectsByAttr("agent", std::nullopt, std::nullopt);
  std::vector<Agent> agents;
  for (auto &id : agentIds){
    agents.push_back(Agent{
      .id = id,
      .type = AGENT_MOVER,
    });
  }

  return agents;
}

struct Goal {
  int goaltype;
  std::any goalData;
  std::function<int(std::any&)> score;
};

std::vector<Goal> getGoalsForAgent(WorldInfo& worldInfo, Agent& agent){
  if (agent.type == AGENT_MOVER){
    std::vector<Goal> goals = {};
    goals.push_back(
      Goal {
        .goaltype = getSymbol("idle"),
        .goalData = NULL,
        .score = [](std::any&) -> int { 
          return 0; 
        }
      }
    );

    std::set<int> symbols = { getSymbol("target") };
    auto targetAttr = getSingleAttr(agent.id, "agent-target");
    if (targetAttr.has_value()){
      symbols.insert(getSymbol(targetAttr.value()));
    }
    auto targetPositions = getVec3StateByTag(worldInfo, symbols);

    if (targetPositions.size() > 0){
      auto targetPosition = targetPositions.at(0);
      goals.push_back(
        Goal {
          .goaltype = getSymbol("move-to-fixed-target-high-value"),
          .goalData = targetPosition,
          .score = [&agent](std::any& targetPosition) -> int { 
            auto targetPos = anycast<glm::vec3>(targetPosition);
            modassert(targetPos, "target pos was null");
            auto distance = glm::distance(*targetPos, gameapi -> getGameObjectPos(agent.id, true));
            auto score = distance > 2 ? 10 : -1; 
            return score;
          }
        }
      );
    
    }
    return goals;
  }
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
  if (goal.goaltype == getSymbol("move-to-fixed-target-high-value")){
    auto targetPosition = anycast<glm::vec3>(goal.goalData);
    modassert(targetPosition, "target pos was null");
    auto agentPos = gameapi -> getGameObjectPos(agent.id, true);
    auto towardTarget = gameapi -> orientationFromPos(agentPos, glm::vec3(targetPosition -> x, agentPos.y, targetPosition -> z));
    auto newPos = gameapi -> moveRelative(agentPos, towardTarget, 1 * gameapi -> timeElapsed());
    gameapi -> setGameObjectPosition(agent.id, newPos, true);  // probably should be applying impulse instead?
    gameapi -> setGameObjectRot(agent.id, towardTarget, true);
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
      modassert(optimalGoal, "no goal for agent");
      if (optimalGoal){
        doGoal(*optimalGoal, agent);  
      }
    }
  };

  binding.onKeyCallback = [](int32_t id, void* data, int key, int scancode, int action, int mods) -> void {
    AiData* aiData = static_cast<AiData*>(data);
    if (key == 'm' && action == 0) { 
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