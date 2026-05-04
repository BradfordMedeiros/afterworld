#ifndef MOD_AFTERWORLD_MODES_MODE
#define MOD_AFTERWORLD_MODES_MODE

#include "./modes/ball.h"
#include "./modes/intro.h"
#include "./modes/fps.h"
#include "./modes/video.h"

struct GameModeNone{};
struct GameModeFps {
  bool makePlayer = false;
  std::optional<std::string> player;
};
struct GameModeBall{};
struct GameModeIntro{};
struct GameModeVideo{};

typedef std::variant<GameModeNone, GameModeFps, GameModeBall, GameModeIntro, GameModeVideo> GameMode;

void startMode(GameMode& gameMode, objid sceneId);
void stopMode(GameMode& gameMode);

void onModeCollision(GameMode& gameMode, objid obj1, objid obj2);


#endif 