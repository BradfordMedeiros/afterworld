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

struct Goal {
  int name;
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

std::vector<Goal> goals = {  // these goals should be able to be dynamic, for example, be constructed by placing two bags of gold and having them pick up the one with higher worth
  Goal { 
    .name = goalSymbol("idle"), 
    .score = [](WorldInfo&) -> int { return 1; }
  },
  Goal { 
    .name = goalSymbol("moveToPlayer"), 
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

Goal& getOptimalGoal(WorldInfo& worldInfo, Agent& agent){
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
  //(define (move-toward targetpos)
  //(define currentpos (gameobj-pos mainobj))
  //(define toward-target (orientation-from-pos currentpos targetpos))
  //(define newpos (move-relative currentpos toward-target (* (time-elapsed) speed-per-second)))
  //(gameobj-setpos! mainobj newpos)
  //(gameobj-setrot! mainobj toward-target)


  auto agentPos = tools.getAgentPosition();
  auto playerPos = tools.getPlayerPosition();
  auto towardTarget = gameapi -> orientationFromPos(agentPos, playerPos);
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
    actionMoveTowardPosition(worldInfo, tools);
    return;
  }
  if (goal.name == shootPlayer){
    actionShootPlayer(worldInfo, tools);
    return;
  }
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
// ~nil:goalname:flag
// ~nil:action:goto, shoot, activate, explore-around
// ~nil:action-value:friendly,100
// ~nil:depends-on:gate-unlocked

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
    auto goal = getOptimalGoal(worldInfo, agent);
    std::cout << "executing goal: " << nameForGoalSymbol(goal.name) << std::endl;
    doGoal(worldInfo, agent, goal, aiData -> tools);
  };

  return binding;
}