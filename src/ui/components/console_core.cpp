#include "./console_core.h"

extern CustomApiBindings* gameapi;

const int CONSOLE_LOG_LIMIT = 25;
const int CONSOLE_DISPLAY_LIMIT = 10;
bool showLog = false;

std::deque<HistoryInstance> loadCommandHistory(){
  std::cout << "load command history" << std::endl;
  auto query = gameapi -> compileSqlQuery("select text, valid from history", {});
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(query, &validSql);
  modassert(validSql, "error executing sql query");

  std::deque<HistoryInstance> history;

  int fromIndex = result.size() - CONSOLE_LOG_LIMIT;
  if (fromIndex < 0){
    fromIndex = 0;
  }

  for (auto &row : result){
    std::cout << "pushing back value: " << row.at(0) << std::endl << std::endl;
  }

  for (int i = fromIndex; i < result.size(); i++){
    auto row = result.at(i);
    std::cout << "pushing back: " << i <<  ", " << row.at(0) << std::endl;
    history.push_back(
      HistoryInstance {
        .command = row.at(0),      
        .valid = row.at(1) == "true",
      }
    );
  }
  return history;
}

void insertCommandHistory(std::string& command, bool valid){
  auto query = gameapi -> compileSqlQuery("insert into history (text, valid) values ( ?, ?)", { command, valid ? "true" : "false" });
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(query, &validSql);
  modassert(validSql, "error executing sql query");
}

std::vector<CommandDispatch> commands {
  CommandDispatch {
    .command = "quit",
    .fn = [](ConsoleInterface& consoleInterface, std::string& commandStr, bool* valid) -> std::optional<std::string> {
      if (commandStr != "quit"){
        *valid = false;
        return std::nullopt;
      }
      exit(1);
      *valid = true;
      return std::nullopt;
    }, 
  },
  CommandDispatch {
    .command = "log",
    .fn = [](ConsoleInterface& consoleInterface, std::string& commandStr, bool* valid) -> std::optional<std::string> {
      showLog = true;
      *valid = true;
      return std::nullopt;
    }, 
  },
  CommandDispatch {
    .command = "history",
    .fn = [](ConsoleInterface& consoleInterface, std::string& commandStr, bool* valid) -> std::optional<std::string> {
      showLog = false;
      *valid = true;
      return std::nullopt;
    }, 
  },
  CommandDispatch {
    .command = "play",
    .fn = [](ConsoleInterface& consoleInterface, std::string& commandStr, bool* valid) -> std::optional<std::string> {
      consoleInterface.setNormalMode();
      return std::nullopt;
    }, 
  },
  CommandDispatch {
    .command = "editor",
    .fn = [](ConsoleInterface& consoleInterface, std::string& commandStr, bool* valid) -> std::optional<std::string> {
      consoleInterface.setShowEditor();
      return std::nullopt;
    }, 
  },
  CommandDispatch {
    .command = "free",
    .fn = [](ConsoleInterface& consoleInterface, std::string& commandStr, bool* valid) -> std::optional<std::string> {
      consoleInterface.setFreeCam();
      return std::nullopt;
    }, 
  },
  CommandDispatch {
    .command = "noclip",
    .fn = [](ConsoleInterface& consoleInterface, std::string& commandStr, bool* valid) -> std::optional<std::string> {
      consoleInterface.setNoClip();
      return std::nullopt;
    }, 
  },
  CommandDispatch {
    .command = "background",
    .fn = [](ConsoleInterface& consoleInterface, std::string& command, bool* valid) -> std::optional<std::string> {
      auto values = split(command, ' ');
      *valid = false;
      if (values.size() != 2){
        return std::nullopt;
      }
      auto backgroundValue = values.at(1);
      consoleInterface.setBackground(backgroundValue);
      *valid = true;
      return std::nullopt;
    },
  },
  CommandDispatch {
    .command = "level",
    .fn = [](ConsoleInterface& consoleInterface, std::string& command, bool* valid) -> std::optional<std::string> {
      *valid = false;
      auto values = split(command, ' ');
      if (values.size() == 1){
        consoleInterface.goToLevel(std::nullopt);
        *valid = true;
        return std::nullopt;
      }
      if (values.size() == 2){
        auto levelName = values.at(1);
        consoleInterface.goToLevel(levelName);
        *valid = true;
        return std::nullopt;
      }
      return std::nullopt;
    },
  },
  CommandDispatch {
    .command = "next",
    .fn = [](ConsoleInterface& consoleInterface, std::string& command, bool* valid) -> std::optional<std::string> {
      *valid = true;
      consoleInterface.nextLevel();
      return std::nullopt;
    },
  },
  CommandDispatch {
    .command = "help",
    .fn = [](ConsoleInterface& consoleInterface, std::string& command, bool* valid) -> std::optional<std::string> {
      *valid = true;
      std::string result = "[commands]\n\n";
      for (auto &command : commands){
        result += command.command + "  ";
      }
      return result;    
    },
  },
  CommandDispatch {
    .command = "router",
    .fn = [](ConsoleInterface& consoleInterface, std::string& command, bool* valid) -> std::optional<std::string> {
      *valid = false;
      auto values = split(command, ' ');
      if (values.size() == 1){
        return std::nullopt;
      }
      if (values.at(1) == "pop"){
        consoleInterface.routerPop();
        return std::nullopt;
      }else if (values.at(1) == "push" && values.size() == 3){
        consoleInterface.routerPush(values.at(2), false);
        return std::nullopt;
      }else if (values.at(1) == "replace"){
        consoleInterface.routerPush(values.at(2), true);
      }

      return std::nullopt;    
    },
  },
  CommandDispatch {
    .command = "marklevel",
    .fn = [](ConsoleInterface& consoleInterface, std::string& command, bool* valid) -> std::optional<std::string> {
      *valid = false;
      auto values = split(command, ' ');
      if (values.size() != 3){
        return std::nullopt;
      }
      auto levelName = values.at(1);
      consoleInterface.markLevelComplete(levelName, 0.f);
      return std::nullopt;    
    },
  },

  CommandDispatch {
    .command = "die",
    .fn = [](ConsoleInterface& consoleInterface, std::string& command, bool* valid) -> std::optional<std::string> {
      *valid = true;
      consoleInterface.die();
      return std::nullopt;    
    },
  },
  CommandDispatch {
    .command = "dump",
    .fn = [](ConsoleInterface& consoleInterface, std::string& command, bool* valid) -> std::optional<std::string> {
      auto values = split(command, ' ');
      if (values.size() == 1){
        gameapi -> debugInfo(std::nullopt);
        exit(1);    
      }

      auto delayMs = std::atoi(values.at(1).c_str());
      gameapi -> schedule(-1, true, delayMs, NULL, [](void*) -> void {
        gameapi -> debugInfo(std::nullopt);
        exit(1);    
      });

      return "dump scheduled";    
    },
  },
  CommandDispatch {
    .command = "keyboard",
    .fn = [](ConsoleInterface& consoleInterface, std::string& command, bool* valid) -> std::optional<std::string> {
      consoleInterface.toggleKeyboard();
      return std::nullopt;    
    },
  },
  CommandDispatch {
    .command = "screenshot",
    .fn = [](ConsoleInterface& consoleInterface, std::string& command, bool* valid) -> std::optional<std::string> {
      auto values = split(command, ' ');
      if (values.size() == 1){
        consoleInterface.takeScreenshot("../afterworld/screenshots/test.png");
        return std::nullopt;
      }
      auto delayMs = std::atoi(values.at(1).c_str());
      auto takeScreenshot = consoleInterface.takeScreenshot;
      gameapi -> schedule(-1, true, delayMs, NULL, [takeScreenshot](void*) -> void {
        takeScreenshot("../afterworld/screenshots/test.png");
      });
      return std::nullopt;    
    },
  },
  CommandDispatch {
    .command = "debugui",
    .fn = [](ConsoleInterface& consoleInterface, std::string& command, bool* valid) -> std::optional<std::string> {
      auto values = split(command, ' ');
      if (values.size() == 1){
        consoleInterface.setShowDebugUi(DEBUG_NONE);
        return std::nullopt;
      }
      auto uiType = values.at(1);
      if (uiType == "global"){
        consoleInterface.setShowDebugUi(DEBUG_GLOBAL);
        return std::nullopt;
      }else if (uiType == "inventory"){
        consoleInterface.setShowDebugUi(DEBUG_INVENTORY);
        return std::nullopt;
      }else if (uiType == "ai"){
        consoleInterface.setShowDebugUi(DEBUG_AI);
        return std::nullopt;
      }else if (uiType == "gametype"){
        consoleInterface.setShowDebugUi(DEBUG_GAMETYPE);
        return std::nullopt;
      }else if (uiType == "health"){
        consoleInterface.setShowDebugUi(DEBUG_HEALTH);
        return std::nullopt;
      }else if (uiType == "player"){
        consoleInterface.setShowDebugUi(DEBUG_ACTIVEPLAYER);
        return std::nullopt;      
      }else if (uiType == "animation"){
        consoleInterface.setShowDebugUi(DEBUG_ANIMATION);
        return std::nullopt;
      }
      return "invalid type";      
    },
  },
  CommandDispatch {
    .command = "hideweapon",
    .fn = [](ConsoleInterface& consoleInterface, std::string& command, bool* valid) -> std::optional<std::string> {
      consoleInterface.showWeapon(false);
      return std::nullopt;
    },
  },
  CommandDispatch {
    .command = "showweapon",
    .fn = [](ConsoleInterface& consoleInterface, std::string& command, bool* valid) -> std::optional<std::string> {
      consoleInterface.showWeapon(true);
      return std::nullopt;
    },
  },
  CommandDispatch {
    .command = "ammo",
    .fn = [](ConsoleInterface& consoleInterface, std::string& command, bool* valid) -> std::optional<std::string> {
      consoleInterface.deliverAmmo(100);
      return std::nullopt;
    },
  },
  CommandDispatch {
    .command = "disable-active",
    .fn = [](ConsoleInterface& consoleInterface, std::string& command, bool* valid) -> std::optional<std::string> {
      consoleInterface.disableActiveEntity(true);
      return std::nullopt;
    },
  },
  CommandDispatch {
    .command = "enable-active",
    .fn = [](ConsoleInterface& consoleInterface, std::string& command, bool* valid) -> std::optional<std::string> {
      consoleInterface.disableActiveEntity(false);
      return std::nullopt;
    },
  },
  CommandDispatch {
    .command = "spawn",
    .fn = [](ConsoleInterface& consoleInterface, std::string& command, bool* valid) -> std::optional<std::string> {
      auto values = split(command, ' ');
      if (values.size() == 2){
        consoleInterface.spawnByTag(values.at(1));
        return std::nullopt;
      }
      return std::nullopt;
    },
  },
  CommandDispatch {
    .command = "speed",
    .fn = [](ConsoleInterface& consoleInterface, std::string& command, bool* valid) -> std::optional<std::string> {
      auto values = split(command, ' ');
      if (values.size() == 1){
        gameapi -> setWorldState({
          ObjectValue {
            .object = "game",
            .attribute = "speed",
            .value = 1.f,
          },
        });  
      }else {
        modassert(values.size() == 2, "speed must be 0 or 1");
        float speed = std::atof(values.at(1).c_str());
        gameapi -> setWorldState({
          ObjectValue {
            .object = "game",
            .attribute = "speed",
            .value = speed,
          },
        });  
      }
      return std::nullopt;
    },
  },
};


std::deque<HistoryInstance> commandHistory =  {};
std::deque<HistoryInstance> logHistory = {};


void initializeConsole(){
  commandHistory = loadCommandHistory();
  gameapi -> setLogEndpoint([](std::string& message) -> void {
    std::cout << message << std::endl;
    if (logHistory.size() >= CONSOLE_LOG_LIMIT){
      logHistory.pop_front();
    }
    logHistory.push_back(HistoryInstance {
      .command = message,
      .valid = true,
    });
  });
}

std::optional<CommandDispatch*> findCommand(std::string commandStr){
  for (auto &command : commands){
    auto splitCommand = split(commandStr, ' ');
    auto mainCommand = splitCommand.at(0);
    if (mainCommand == command.command){
      return &command;
    }
  }
  return std::nullopt;
}


void executeCommand(ConsoleInterface& consoleInterface, std::string command){
  if (commandHistory.size() >= CONSOLE_LOG_LIMIT){
    commandHistory.pop_front();
  }

  std::transform(command.begin(), command.end(), command.begin(), [](unsigned char c) {
    return std::tolower(c);
  });
  auto commandDispatch = findCommand(command);
  if (commandDispatch.has_value()){
    bool valid = false;
    auto result = commandDispatch.value() -> fn(consoleInterface, command, &valid);
    commandHistory.push_back(HistoryInstance {
      .command = command,
      .valid = valid,
    });
    insertCommandHistory(command, valid);

    if (result.has_value()){
      commandHistory.push_back(HistoryInstance {
        .command = result.value(),
        .valid = valid,
      });
    }

  }else{
    commandHistory.push_back(HistoryInstance {
      .command = command,
      .valid = false,
    });
    insertCommandHistory(command, false);
  }
}
