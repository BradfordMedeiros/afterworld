#include "./options.h"

extern CustomApiBindings* gameapi;

std::vector<GameOption> gameOptions {
  GameOption {
    .arg = "help",
    .description = "print extended help options",
    .option = BoolOption{},
  },
  GameOption {
    .arg = "debug-shoot",
    .description = "draw debug vector for shooting",
    .option = BoolOption{},
  },
  GameOption {
    .arg = "print-type",
    .description = "print extended help options",
    .option = ChoiceOption{ 
      .options = { "inventory", "global", "gametype", "ai", "health", "active", "animation" }
    },
  },
  GameOption {
    .arg = "level",
    .description = "initial level to load into",
    .option = StrOption{},
  },
  GameOption {
    .arg = "route",
    .description = "initial route to load",
    .option = StrOption{},
  }, 
  GameOption {
    .arg = "debug-ui",
    .description = "debug information for the ui",
    .option = BoolOption{},
  },
  GameOption {
    .arg = "no-mesh-tp",
    .description = "disable third person mesh for controlled entities",
    .option = BoolOption{},
  },
  GameOption {
    .arg = "no-animation",
    .description = "disable animation controller animations",
    .option = BoolOption{},
  },
  GameOption {
    .arg = "no-ai",
    .description = "disable ai",
    .option = BoolOption{},
  },
  GameOption {
    .arg = "validate-animation",
    .description = "validate state controller animations to verify they exist",
    .option = BoolOption{},
  },
  GameOption {
    .arg = "dev",
    .description = "misc options and controls for development mode",
    .network = NetworkOption {
      .target = "mode",
      .attribute = "dev",
    },
  },
  GameOption {
    .arg = "arcade",
    .description = "play arcade game instead, full screen",
    .option = StrOption{}, 
  },
  GameOption {
    .arg = "background",
    .description = "texture to use for the background",
    .option = StrOption{}, 
    .network = NetworkOption {
      .target = "background", 
      .attribute = "image",
    },
  },
  GameOption {
    .arg = "config-server",
    .description = "server to connect to for configuration",
    .option = StrOption{}, 
  },
  GameOption {
    .arg = "title",
    .description = "title to put on the main menu instead of Afterworld",
    .option = StrOption{},
    .network = NetworkOption {
      .target = "title", 
      .attribute = "name",
    }, 
  },
  GameOption {
    .arg = "motd",
    .description = "message of the day text to display on the menu",
    .option = StrOption{},
    .network = NetworkOption {
      .target = "motd", 
      .attribute = "text",
    }, 
  },
  GameOption {
    .arg = "motd-image",
    .description = "message of the day background texture",
    .option = StrOption{},
    .network = NetworkOption {
      .target = "motd", 
      .attribute = "image",
    }, 
  },
  GameOption {
    .arg = "remote-mod",
    .description = "path of a remote mod package to install. This gets appended to the server url",
    .option = StrOption{},
    .network = NetworkOption {
      .target = "mod", 
      .attribute = "path",
    }, 
  },
};

std::unordered_map<std::string, std::string>& getArgData(){
  static bool loadedOnce = false;
  static std::unordered_map<std::string, std::string> argData;
  if (!loadedOnce){
    loadedOnce = true;
    argData = gameapi -> getArgs();

    std::optional<std::string> configServer;
    if (argData.find("config-server") != argData.end()){
      auto serverAddr = argData.at("config-server");
      auto isOnline = gameapi -> isServerOnline(serverAddr);
      if (isOnline){
        argData["config-connected"] = "true";
      }
      if (isOnline){
        modlog("server config", "server online");
        bool success = false;
        auto config = gameapi -> getServerConfig(serverAddr + "/game/game.txt", &success); // TODO make path more robust
        modlog("server config", "config get success");
        if (success && config.has_value()){
          for (auto &option : gameOptions){
            if (argData.find(option.arg) != argData.end()){
              continue;
            }
            if (option.network.has_value()){
              auto propertyValue = gameapi -> getProperty(config.value(), option.network.value().target, option.network.value().attribute);
              if (propertyValue.has_value()){
                modlog("server config add option", option.arg);
                argData[option.arg] = propertyValue.value();
              }
            }
          }
        }

        //
        //if (backgroundImage.has_value()){
        //  modlog("server config background image", backgroundImage.value());
        //}else{
        //  modlog("server config background image", "missing property");
        //}
      }
    }
  }
  return argData;
} 

bool getArgEnabled(const char* name){
  auto args = getArgData();
  return args.find(name) != args.end();
}
bool getArgEqual(const char* name, const char* value){
  auto args = getArgData();
  if (args.find(name) == args.end()){
    return false;
  }
  return args.at(name) == value;
}

bool getArgChoice(const char* name, const char* choice){
  auto args = getArgData();
  if (args.find(name) == args.end()){
    return false;
  }
  return args.at(name) == choice;
}

std::string getArgOption(const char* name){
  auto args = getArgData();
	return args.at(name);
}

bool hasOption(const char* name){
  auto args = getArgData();
  return args.find(name) != args.end();
}

std::string helpToStr(GameOption& gameOption){
  std::string value = gameOption.arg + ", ";

  auto boolType = std::get_if<BoolOption>(&gameOption.option);
  auto choiceType = std::get_if<ChoiceOption>(&gameOption.option);
  auto strType = std::get_if<StrOption>(&gameOption.option);
  if (boolType){
    value += "type = bool";
  }else if (choiceType){
    value += "type = choice [ ";
    for (auto &option : choiceType -> options){
      value += option + " ";
    }
    value += "]";

  }else if (strType){
    value += "type = str";
  }else{
    modassert(false, "invalid option type");
  }

  value += ", " + gameOption.description;
  return value;
}


void printGameOptionsHelp(){
	for (auto &gameOption : gameOptions){
    std::cout << helpToStr(gameOption) << std::endl;
  }
}