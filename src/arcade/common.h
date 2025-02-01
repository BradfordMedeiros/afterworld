#ifndef MOD_AFTERWORLD_ARCADE_COMMON
#define MOD_AFTERWORLD_ARCADE_COMMON

struct ArcadeInterface {
  std::function<std::any()> createInstance;
  std::function<void(std::any&)> rmInstance;
  std::function<void(std::any&)> update;
  std::function<void(std::any&, std::optional<objid> textureId)> draw;
  std::function<void(std::any&, int key, int scancode, int action, int mod)> onKey;
  std::function<void(std::any&, double xPos, double yPos, float xNdc, float yNdc)> onMouseMove;
};


#endif