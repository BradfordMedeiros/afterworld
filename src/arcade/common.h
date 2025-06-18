#ifndef MOD_AFTERWORLD_ARCADE_COMMON
#define MOD_AFTERWORLD_ARCADE_COMMON

#include "../../../ModEngine/src/cscript/cscript_binding.h"
#include "../util.h"
#include "../resources/paths.h"

struct ArcadeInterface {
  std::function<std::any(objid)> createInstance;
  std::function<void(std::any&)> rmInstance;
  std::function<void(std::any&)> update;
  std::function<void(std::any&, std::optional<objid> textureId)> draw;
  std::function<void(std::any&, int key, int scancode, int action, int mod)> onKey;
  std::function<void(std::any&, double xPos, double yPos, float xNdc, float yNdc)> onMouseMove;
  std::function<void(std::any&, int button, int action, int mods)> onMouseClick;
  std::function<void(std::any&)> onMessage;
};

struct ArcadeApi {
  std::function<std::vector<objid>(objid, std::vector<std::string>)> ensureSoundsLoaded;
  std::function<void(objid)> releaseSounds;

  std::function<void(objid, std::vector<std::string>)> ensureTexturesLoaded;
  std::function<void(objid)> releaseTextures;

  std::function<void(objid)> playSound;
  std::function<glm::vec2(objid)> getResolution;
};

struct Collider2DRect {
  objid id;
  glm::vec2 position;
  glm::vec2 size;
};
struct Collision2DArcade {
  std::vector<Collider2DRect> objs;
};
struct Collision2D {
  objid obj1;
  objid obj2;
};

Collision2DArcade create2DCollisions();
void addColliderRect(Collision2DArcade& collision, objid id, glm::vec2 pos, glm::vec2 size);
void rmCollider(Collision2DArcade& collision, objid id);
void updatePosition(Collision2DArcade& collision, objid id, glm::vec2 pos);
std::vector<Collision2D> getCollisionsArcade(Collision2DArcade& collision);
void drawCollisionDebug(Collision2DArcade& collision, std::optional<objid> textureId);

glm::vec2 rotatePoint(glm::vec2 point, glm::vec2 dir);
void drawCenteredTextFade(const char* text, float x, float y, float size, glm::vec3 color, float period, std::optional<objid> textureId);

#endif