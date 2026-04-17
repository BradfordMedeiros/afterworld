#ifndef MOD_AFTERWORLD_ROUTING
#define MOD_AFTERWORLD_ROUTING

#include <string>
#include <vector>
#include <optional>
#include "./ui/components/router.h"
#include "./gamecontrol/mode.h"

struct PathAndParams {
  std::string path;
  std::vector<std::string> params;
};
struct SceneRouterOptions {
  std::vector<PathAndParams> paths;
  bool paused;
  bool inGameMode;
  bool showMouse;
};

std::optional<SceneRouterOptions*> getRouterOptions(std::string& path, std::vector<std::string>& queryParams, int * _index);

struct SceneLoadInfo {
  std::string sceneFile;
  std::vector<std::vector<std::string>> additionalTokens;
};
struct ScenarioOptions {
  glm::vec3 ambientLight;
  glm::vec3 skyboxColor;
  std::string skybox;
  std::string audioClipPath;
};
struct SceneRouterPath {
  std::vector<std::string> paths;
  std::optional<std::function<SceneLoadInfo(std::vector<std::string> params)>> scene;
  std::optional<std::function<ScenarioOptions(std::vector<std::string> params)>> scenarioOptions;
  std::function<GameMode(std::vector<std::string> params)> getGameMode = [](std::vector<std::string> params) -> GameMode { return GameModeNone{}; };
};


std::optional<SceneRouterPath*> getSceneRouter(std::string& path, int* _index, std::vector<std::string>* _params);


#endif 