#ifndef MOD_AFTERWORLD_MODES_MODE
#define MOD_AFTERWORLD_MODES_MODE

#include "./modes/ball.h"
#include "./modes/intro.h"
#include "./modes/fps.h"

struct GameModeNone{};
struct GameModeFps {
  bool makePlayer = false;
  std::optional<std::string> player;
};
struct GameModeBall{};
struct GameModeIntro{};

typedef std::variant<GameModeNone, GameModeFps, GameModeBall, GameModeIntro> GameMode;

void startMode(GameMode& gameMode, objid sceneId);
void stopMode(GameMode& gameMode);



#endif 