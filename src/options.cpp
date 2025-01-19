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
    .arg = "validate-animation",
    .description = "validate state controller animations to verify they exist",
    .option = BoolOption{},
  },

  GameOption {
    .arg = "dev",
    .description = "misc options and controls for development mode",
  },

};




bool getArgEnabled(const char* name){
  auto args = gameapi -> getArgs();
  return args.find(name) != args.end();
}
bool getArgEqual(const char* name, const char* value){
  auto args = gameapi -> getArgs();
  if (args.find(name) == args.end()){
    return false;
  }
  return args.at(name) == value;
}

bool getArgChoice(const char* name, const char* choice){
  auto args = gameapi -> getArgs();
  if (args.find(name) == args.end()){
    return false;
  }
  return args.at(name) == choice;
}

std::string getArgOption(const char* name){
  auto args = gameapi -> getArgs();
	return args.at(name);
}

bool hasOption(const char* name){
  auto args = gameapi -> getArgs();
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