#ifndef MOD_AFTERWORLD_MODES_MODE
#define MOD_AFTERWORLD_MODES_MODE

#include "./modes/ball.h"
#include "./modes/fps.h"
#include "./modes/video.h"
#include "./modes/boot.h"

struct GameModeNone{};
struct GameModeFps {
  bool makePlayer = false;
  std::optional<std::string> player;
};
struct GameModeBall{};
struct GameModeVideo{};
struct GameModeBoot{};

typedef std::variant<GameModeNone, GameModeFps, GameModeBall, GameModeVideo, GameModeBoot> GameMode;

void startMode(GameMode& gameMode, objid sceneId);
void stopMode(GameMode& gameMode);
bool isModeNone(GameMode& gameMode);

void onModeCollision(GameMode& gameMode, objid obj1, objid obj2);


#endif 