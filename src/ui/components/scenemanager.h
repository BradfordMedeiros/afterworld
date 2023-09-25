#ifndef MOD_AFTERWORLD_COMPONENTS_SCENEMANAGER
#define MOD_AFTERWORLD_COMPONENTS_SCENEMANAGER

#include "./common.h"
#include "./basic/layout.h"
#include "./basic/listitem.h"
#include "./basic/list.h"

struct SceneManagerInterface {
  bool showScenes;
  int offset;
  std::function<void(int, std::string)> onSelectScene;
  std::function<void()> toggleShowScenes;
  std::vector<std::string> scenes;
  int currentScene;
};


extern Component scenemanagerComponent;

#endif