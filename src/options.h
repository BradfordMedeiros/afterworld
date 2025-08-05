#ifndef MOD_AFTERWORLD_OPTIONS
#define MOD_AFTERWORLD_OPTIONS

#include <variant>
#include <vector>
#include <string>
#include "./util.h"

struct BoolOption { };
struct ChoiceOption {
  std::vector<std::string> options;
};
struct StrOption {};
typedef std::variant<BoolOption, ChoiceOption, StrOption> GameOptionType;


struct NetworkOption {
  std::string target;
  std::string attribute;
};
struct GameOption {
  std::string arg;
  std::string description;
  GameOptionType option;
  std::optional<NetworkOption> network;
};

bool getArgEnabled(const char* name);
bool getArgEqual(const char* name, const char* value);
bool getArgChoice(const char* name, const char* choice);
std::string getArgOption(const char* name);
bool hasOption(const char* name);
std::string helpToStr(GameOption& gameOption);
void printGameOptionsHelp();

#endif