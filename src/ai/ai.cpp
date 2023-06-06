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

struct BoolState { 
  int symbol;
  bool value;
};
struct Vec3State {
  int symbol;
  glm::vec3 value;
};

struct WorldInfo {
  std::vector<BoolState> boolValues;
  std::vector<Vec3State> vec3Values;
};

struct AiData {
  WorldInfo worldInfo;
};



void updateBoolState(WorldInfo& worldInfo, std::string name, bool value){
  for (auto &boolValue : worldInfo.boolValues){
    if (boolValue.symbol == getSymbol(name)){
      boolValue.value = value;
      return;
    }
  }
  worldInfo.boolValues.push_back(BoolState {
    .symbol = getSymbol(name),
    .value = value,
  });
}
void updateVec3State(WorldInfo& worldInfo, std::string name, glm::vec3 value){
  for (auto &vec3Value : worldInfo.vec3Values){
    if (vec3Value.symbol == getSymbol(name)){
      vec3Value.value = value;
      return;
    }
  }
  worldInfo.vec3Values.push_back(Vec3State {
    .symbol = getSymbol(name),
    .value = value,
  });
}

void detectWorldInfo(WorldInfo& worldInfo){
  updateBoolState(worldInfo, "is-night-time", false);
  updateBoolState(worldInfo, "is-day-time", true);
  updateVec3State(worldInfo, "target-position", glm::vec3(0, 1, 1));
}

std::optional<glm::vec3> getVec3State(WorldInfo& worldInfo, int symbol){
  for (auto &vec3Value : worldInfo.vec3Values){
    if (vec3Value.symbol == symbol){
      return vec3Value.value;
    }
  }
  return std::nullopt;
}

void printWorldInfo(WorldInfo& worldInfo){
  std::cout << "world info: [" << std::endl;

  std::cout << "  bool = [" << std::endl;
  for (auto &boolValue : worldInfo.boolValues){
    std::cout << "    [" << nameForSymbol(boolValue.symbol) << ", " << print(boolValue.value) << "]" << std::endl;
  }
  std::cout << "  ]" << std::endl;

  std::cout << "  vec3 = [" << std::endl;
  for (auto &vec3Value : worldInfo.vec3Values){
    std::cout << "    [" << nameForSymbol(vec3Value.symbol) << ", " << print(vec3Value.value) << "]" << std::endl;
  }
  std::cout << "  ]" << std::endl;

  std::cout << "]" << std::endl;
}

enum AgentType { AGENT_MOVER };
struct Agent { 
  objid id;
  AgentType type;
};

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
    auto targetPosition = getVec3State(worldInfo, getSymbol("target-position"));
    if (targetPosition.has_value()){
      goals.push_back(
        Goal {
          .goaltype = getSymbol("idle"),
          .goalData = targetPosition.value(),
          .score = [](std::any&) -> int { 
            return 0; 
          }
        }
      );
      goals.push_back(
        Goal {
          .goaltype = getSymbol("move-to-fixed-target-high-value"),
          .goalData = targetPosition.value(),
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
    detectWorldInfo(aiData -> worldInfo);

    for (auto &agent : createAgents()){
      auto goals = getGoalsForAgent(aiData -> worldInfo, agent);
      auto optimalGoal = getOptimalGoal(goals);
      doGoal(*optimalGoal, agent);  
    }


  };

  binding.onKeyCallback = [](int32_t id, void* data, int key, int scancode, int action, int mods) -> void {
    AiData* aiData = static_cast<AiData*>(data);
    if (key == '.' && action == 0) { 
      printWorldInfo(aiData -> worldInfo);
    }

    if (key == '.' && action == 0){
      auto spawnpointIds  = gameapi -> getObjectsByAttr("spawn", std::nullopt, std::nullopt);
      for (auto spawnpointId : spawnpointIds){
        spawnPlayer(spawnpointId);
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