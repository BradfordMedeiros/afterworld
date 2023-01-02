#include "./ai.h"

extern CustomApiBindings* gameapi;

/*
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

Goal createIdleGoal(std::string name, int score){
  return Goal { 
    .name = goalSymbol(name), 
    .typeValue = IdleGoal {},
    .score = [score](WorldInfo&) -> int { return score; }
  };
}

Goal createMoveToGoal(std::string name, int score){
  return Goal { 
    .name = goalSymbol(name), 
    .typeValue = MoveToGoal { .getPosition = []() -> glm::vec3 { return glm::vec3(0.f, 0.f, 0.f); }},
    .score = [score](WorldInfo&) -> int { return score; }
  };
}
Goal createAttackGoal(std::string name){
  return Goal { 
    .name = goalSymbol(name), 
    .typeValue = AttackGoal {},
    .score = [](WorldInfo&) -> int { return 0.f; }
  };
}
Goal createActivateGoal(std::string name){
  return Goal { 
    .name = goalSymbol(name), 
    .typeValue = ActivateGoal {},
    .score = [](WorldInfo&) -> int { return 0.f; }
  };
}
Goal createExploreGoal(std::string name){
  return Goal { 
    .name = goalSymbol(name), 
    .typeValue = ExploreGoal {},
    .score = [](WorldInfo&) -> int { return 0.f; }
  };
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

void actionMoveTowardPosition(WorldInfo& worldInfo, AiTools& tools){

}

void actionMoveTowardPlayer(WorldInfo& worldInfo, AiTools& tools){
  auto agentPos = tools.getAgentPosition();
  auto playerPos = tools.getPlayerPosition();
  auto towardTarget = gameapi -> orientationFromPos(agentPos, glm::vec3(playerPos.x, agentPos.y, playerPos.z));
  auto newPos = gameapi -> moveRelative(agentPos, towardTarget, 1 * gameapi -> timeElapsed());
  tools.setAgentPosition(newPos);
  tools.setAgentRotation(towardTarget);
}

void actionShootPlayer(WorldInfo& worldInfo, AiTools& tools){

}

void doGoal(WorldInfo& worldInfo, Agent& agent, Goal& goal, AiTools& tools){
  static int idle = goalSymbol("idle");
  static int moveToPlayer = goalSymbol("moveToPlayer");
  static int shootPlayer = goalSymbol("shootPlayer");
  if (goal.name == idle){
    // do nothing
    return;
  }
  if (goal.name == moveToPlayer){
    actionMoveTowardPlayer(worldInfo, tools);
    return;
  }
  if (std::get_if<MoveToGoal>(&goal.typeValue) != NULL){
    return;
  }

  if (goal.name == shootPlayer){
    actionShootPlayer(worldInfo, tools);
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
  std::cout << "goal objects size: " << goalObjects.size() << std::endl;

  for (auto id : goalObjects){
    auto objAttr =  gameapi -> getGameObjectAttr(id);
    auto goalName = getStrAttr(objAttr, "goal");
    auto goalType = getStrAttr(objAttr, "goal-type").value();
    auto optGoalValue = getFloatAttr(objAttr, "goal-value");
    auto goalValue = optGoalValue.has_value() ? optGoalValue.value() : 1;
    std::cout << "goal: name = " << goalName.value() << ", type = " << goalType << ", value = " << goalValue << std::endl;

    if (goalType == "idle"){
      goals.push_back(createIdleGoal(goalName.value(), goalValue));
    }else if (goalType == "move"){
      goals.push_back(createMoveToGoal(goalName.value(), goalValue));
    }else if (goalType == "explore"){
      goals.push_back(createExploreGoal(goalName.value()));
    }else if (goalType == "attack"){
      goals.push_back(createAttackGoal(goalName.value()));
    }else if (goalType == "activate"){
      goals.push_back(createActivateGoal(goalName.value()));
    }else{
      modassert(false, "invalid goal type");
    }
  }

}

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
        gameapi -> setGameObjectPosRelative(id, pos);
      },
      .setAgentRotation = [id](glm::quat rot) -> void {
        gameapi -> setGameObjectRot(id, rot);
      },
    };

    aiData -> goals = {  // these goals should be able to be dynamic, for example, be constructed by placing two bags of gold and having them pick up the one with higher worth
      createIdleGoal("idle", 1),
      Goal { 
        .name = goalSymbol("moveToPlayer"),
        //.conditions = { GoalCondition { .key = goalSymbol("canSeePlayer"), .value = true } }, 
        .score = [](WorldInfo& info) -> int { 
          if (!info.canSeePlayer){
            return 0;
          }
          return 10;
        }
      },
      Goal {
        .name = goalSymbol("shootPlayer"),
        .score = [](WorldInfo& info) -> int {
          if (info.canShootPlayer){
            return 20;
          }else{
            return 0;  // alternatively maybe make this have a dependency on goal moveToPlayer (and add some idea of goal being completed)
          }
        }
      },
      Goal {
        .name = goalSymbol("explore"),
        .score = [](WorldInfo& info) -> int {
          return info.isAlerted ? 5 : 0;
        }
      },
    };
    addGoals(aiData -> goals);

    return aiData;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
    AiData* aiData = static_cast<AiData*>(data);
    delete aiData;
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