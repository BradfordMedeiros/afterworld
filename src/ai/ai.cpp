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


struct WorldInfo {
  bool canSeePlayer;
  bool canShootPlayer;
  bool isAlerted;
  glm::vec3 playerPosition;
  // can i include the goals themselves here as available state? 
};

struct Agent {
  objid id;
};

struct IdleGoal { };
struct MoveToGoal { std::function<glm::vec3()> getPosition; };
struct ExploreGoal { };
struct AttackGoal { };
struct ActivateGoal { };
typedef std::variant<IdleGoal, MoveToGoal, ExploreGoal, AttackGoal, ActivateGoal> GoalTypeValue;


std::string goalTypeToStr(GoalTypeValue& type){
  if (std::get_if<IdleGoal>(&type) != NULL){
    return "idle";
  }else if (std::get_if<MoveToGoal>(&type) != NULL){
    return "move";
  }else if (std::get_if<ExploreGoal>(&type) != NULL){
    return "explore";
  }else if (std::get_if<AttackGoal>(&type) != NULL){
    return "attack";
  }else if (std::get_if<ActivateGoal>(&type) != NULL){
    return "activate";
  }
  modassert(false, "invalid goal type");
  return "";
}


struct Goal {
  int name;
  GoalTypeValue typeValue;
  std::function<int(WorldInfo&)> score;
};


std::unordered_map<std::string, int> goalnameToInt;

struct AiTools {
  std::function<glm::vec3(void)> getPlayerPosition;
  std::function<glm::vec3(void)> getAgentPosition;
  std::function<void(glm::vec3)> setAgentPosition;
  std::function<void(glm::quat)> setAgentRotation;
};
struct AiData {
  AiTools tools;
  std::vector<Goal> goals;
};


int symbolIndex = -1;
int goalSymbol(std::string name){
  if (goalnameToInt.find(name) != goalnameToInt.end()){
    return goalnameToInt.at(name);
  }
  symbolIndex++;
  goalnameToInt[name] = symbolIndex;
  return symbolIndex;
} 

std::string nameForGoalSymbol(int symbol){
  for (auto &[name, symbolIndex] : goalnameToInt){
    if (symbolIndex == symbol){
      return name;
    }
  }
  modassert(false, "could not find symbol for: " + symbol);
  return "";
}

Goal& getOptimalGoal(std::vector<Goal>& goals, WorldInfo& worldInfo, Agent& agent){
  modassert(goals.size() > 0, "no goals in list for agent");
  int maxScore = goals.at(0).score(worldInfo);
  int maxScoreIndex = 0;
  for (int i = 1; i < goals.size(); i++){
    auto goalScore = goals.at(i).score(worldInfo);
    if (goalScore > maxScore){
      maxScore = goalScore;
      maxScoreIndex = i;
    }
  }
  return goals.at(maxScoreIndex);
}

void moveTo(AiTools& tools, glm::vec3 playerPos){
  auto agentPos = tools.getAgentPosition();
  auto towardTarget = gameapi -> orientationFromPos(agentPos, glm::vec3(playerPos.x, agentPos.y, playerPos.z));
  auto newPos = gameapi -> moveRelative(agentPos, towardTarget, 1 * gameapi -> timeElapsed());
  tools.setAgentPosition(newPos);
  tools.setAgentRotation(towardTarget);
}
void actionMoveTo(WorldInfo& worldInfo, AiTools& tools, MoveToGoal& moveToGoal){
  moveTo(tools, moveToGoal.getPosition());
}

void actionShootPlayer(WorldInfo& worldInfo, AiTools& tools){

}

void doGoal(WorldInfo& worldInfo, Agent& agent, Goal& goal, AiTools& tools){
  auto idleGoal = std::get_if<IdleGoal>(&goal.typeValue);
  if (idleGoal != NULL){
    return;
  }
  auto moveToGoal = std::get_if<MoveToGoal>(&goal.typeValue);
  if (moveToGoal != NULL){
    actionMoveTo(worldInfo, tools, *moveToGoal);
    return;
  }

  modassert(false, "invalid goal name: " + nameForGoalSymbol(goal.name));
}


bool canSeePlayer(AiTools& tools){
  auto agentPos = tools.getAgentPosition();
  auto playerPos = tools.getPlayerPosition();
  return glm::length(agentPos - playerPos) < 20;
}
bool canShootPlayer(AiTools& tools){
  auto agentPos = tools.getAgentPosition();
  auto playerPos = tools.getPlayerPosition();
  return glm::length(agentPos - playerPos) < 10;
}

glm::vec3 getPlayerPosition(AiTools& tools){
  return tools.getPlayerPosition();
}



void detectWorldInfo(WorldInfo& worldInfo, AiTools& tools){
  worldInfo.playerPosition = getPlayerPosition(tools);
  worldInfo.canSeePlayer = canSeePlayer(tools);
  worldInfo.canShootPlayer = canShootPlayer(tools);
  worldInfo.isAlerted |= worldInfo.canSeePlayer;
}


// insert goals objects into the scene that exist
// ~nil:goal:flag
// ~nil:type:goto, shoot, activate, explore-around
// ~nil:action-value:friendly,100
// ~nil:depends-on:gate-unlocked

void addGoals(std::vector<Goal>& goals){ // this needs to be updated during the game too 
  auto goalObjects = gameapi -> getObjectsByAttr("goal", std::nullopt, std::nullopt);
  modlog("ai", "goal objects size: " + std::to_string(goalObjects.size()));

  for (auto id : goalObjects){
    auto objAttr =  gameapi -> getGameObjectAttr(id);
    auto goalName = getStrAttr(objAttr, "goal");
    auto goalType = getStrAttr(objAttr, "goal-type").value();
    auto optGoalValue = getFloatAttr(objAttr, "goal-value");
    auto goalValue = optGoalValue.has_value() ? optGoalValue.value() : 1;
    modlog("ai", "goal: name = " + goalName.value() + ", type = " + goalType + ", value = " + std::to_string(goalValue));

    if (goalType == "idle"){
      goals.push_back(
        Goal { 
          .name = goalSymbol(goalName.value()), 
          .typeValue = IdleGoal {},
          .score = [goalValue](WorldInfo&) -> int { return goalValue; }
        }
      );
    }else if (goalType == "move"){
      goals.push_back(
        Goal { 
          .name = goalSymbol(goalName.value()), 
          .typeValue = MoveToGoal { .getPosition = [id]() -> glm::vec3 { return gameapi -> getGameObjectPos(id, true); }},
          .score = [goalValue](WorldInfo&) -> int { return goalValue; }
        }
      );
    }else if (goalType == "explore"){
      goals.push_back(
        Goal { 
          .name = goalSymbol(goalName.value()), 
          .typeValue = ExploreGoal {},
          .score = [](WorldInfo&) -> int { return 0.f; }
        }
      );
    }else if (goalType == "attack"){
      goals.push_back(
        Goal { 
          .name = goalSymbol(goalName.value()), 
          .typeValue = AttackGoal {},
          .score = [](WorldInfo&) -> int { return 0.f; }
        }
      );
    }else if (goalType == "activate"){
      goals.push_back(
        Goal { 
            .name = goalSymbol(goalName.value()), 
            .typeValue = ActivateGoal {},
            .score = [](WorldInfo&) -> int { return 0.f; }
        }
      );
    }else{
      modassert(false, "invalid goal type");
    }
  }
}

//enemy:tint:1 0 0 1
//enemy:goal:idle-enemy
//enemy:goal-type:idle
//enemy:goal-value:2
//enemy:agent:true
//enemy:script:native/ai
//enemy:health:130
//enemy:team:blue


CScriptBinding aiBinding(CustomApiBindings& api, const char* name){
  auto binding = createCScriptBinding(name, api);
  binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    AiData* aiData = new AiData;
    auto playerId = gameapi -> getGameObjectByName(">maincamera", sceneId, false);
    modassert(playerId.has_value(), "ai - player id needs to have value");

    aiData -> tools = AiTools {
      .getPlayerPosition = [playerId](void) -> glm::vec3 {
        return gameapi -> getGameObjectPos(playerId.value(), true);
      },
      .getAgentPosition = [id](void) -> glm::vec3 {
        return gameapi -> getGameObjectPos(id, true);
      },
      .setAgentPosition = [id](glm::vec3 pos) -> void {
        gameapi -> setGameObjectPosition(id, pos, false);
      },
      .setAgentRotation = [id](glm::quat rot) -> void {
        gameapi -> setGameObjectRot(id, rot, false);
      },
    };

    //ScorerFn = { SimplerScore { Key: getSymboL("canSeePlayer"), Multipler: 0  }};

    aiData -> goals = {  // these goals should be able to be dynamic, for example, be constructed by placing two bags of gold and having them pick up the one with higher worth
      //Goal { 
      //  .name = goalSymbol("moveToPlayer"),
      //  .typeValue = MoveToGoal { .getPosition = [aiData]() -> glm::vec3 { return aiData -> tools.getPlayerPosition(); }},
      //  //.conditions = { GoalCondition { .key = goalSymbol("canSeePlayer"), .value = true } }, 
      //  .score = [](WorldInfo& info) -> int { 
      //    if (!info.canSeePlayer){
      //      return 0;
      //    }
      //    return 10;
      //  }
      //},
      //Goal {
      //  .name = goalSymbol("shootPlayer"),
      //  .score = [](WorldInfo& info) -> int {
      //    if (info.canShootPlayer){
      //      return 20;
      //    }else{
      //      return 0;  // alternatively maybe make this have a dependency on goal moveToPlayer (and add some idea of goal being completed)
      //    }
      //  }
      //},
      //Goal {
      //  .name = goalSymbol("explore"),
      //  .score = [](WorldInfo& info) -> int {
      //    return info.isAlerted ? 5 : 0;
      //  }
      //},
    };
    addGoals(aiData -> goals);

    return aiData;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
    AiData* aiData = static_cast<AiData*>(data);
    delete aiData;
  };

  binding.onMessage = [](int32_t id, void* data, std::string& key, std::any& value){
    if (key == "spawn"){
      auto spawnId = gameapi -> getGameObjectByName("spawnpoint", gameapi -> listSceneId(id), false).value();
      spawnPlayer(spawnId);
    }
  };

  binding.onKeyCallback = [](int32_t id, void* data, int key, int scancode, int action, int mods) -> void {
    if (key == ';' && action == 1){
      std::cout << "add spawn point" << std::endl;
      gameapi -> sendNotifyMessage("spawn", 0);
    }else if (key == '[' && action == 1){
      std::cout << "player holder print goals" << std::endl;
    }
  };

  binding.onFrame = [](int32_t id, void* data) -> void {
    AiData* aiData = static_cast<AiData*>(data);
    Agent agent { 
      .id = id 
    };
    WorldInfo worldInfo {};

    detectWorldInfo(worldInfo, aiData -> tools);
    auto goal = getOptimalGoal(aiData -> goals, worldInfo, agent);
    std::cout << "executing goal: " << nameForGoalSymbol(goal.name) << std::endl;
    doGoal(worldInfo, agent, goal, aiData -> tools);
  };

  return binding;
}