#include "./interfaces.h"

void ensureManagedTexturesLoaded(objid id, objid sceneId, std::vector<std::string> textures);
void unloadManagedTexturesLoaded(objid id);

extern MovementEntityData movementEntities;
extern Weapons weapons;

ArcadeApi createArcadeApi(){
  ArcadeApi arcadeApi {
    .ensureTexturesLoaded = [](objid id, std::vector<std::string> textures) -> void {
      ensureManagedTexturesLoaded(id, rootSceneId(), textures);
    },
    .releaseTextures = unloadManagedTexturesLoaded,
    .playSound = [](objid clipId) -> void {
      playGameplayClipById(clipId, std::nullopt, std::nullopt, false);
    },
    .getResolution = [](objid id) -> glm::vec2 {
      auto texture = arcadeTextureId(id);
      if (texture.has_value()){
        return glm::vec2(1000, 1000); // this is overly coupled to the create texture call in tags
      }
      auto resolutionAttr = getWorldStateAttr("rendering", "resolution").value();
      glm::vec2* resolution = std::get_if<glm::vec2>(&resolutionAttr);
      modassert(resolution, "resolution value invalid");
      return *resolution;
    }
  };
  return arcadeApi;
}

AIInterface aiInterface {
  .move = [](objid agentId, glm::vec3 targetPosition, float speed) -> void {
    setEntityTargetLocation(movementEntities, agentId, MovementRequest {
      .position = targetPosition,
      .speed = speed * 0.6f,
    });
  },
  .stopMoving = [](objid agentId) -> void {
    setEntityTargetLocation(movementEntities, agentId, std::nullopt);
  },
  .look = [](objid agentId, glm::quat direction) -> void {
    setEntityTargetRotation(movementEntities, agentId, direction);
  },
  .fireGun = [](objid agentId) -> void {
    fireGun(weapons, agentId);
  },
  .changeGun = [](objid agentId, const char* gun) -> void {
    maybeChangeGun(getWeaponState(weapons, agentId), gun,  agentId /*inventory */);
  },
  .changeTraits = [](objid agentId, const char* profile) -> void {
    changeMovementEntityType(movementEntities, agentId, profile);
  },
  .playAnimation = [](objid agentId, const char* animation, AnimationType animationType){
    gameapi -> playAnimation(agentId, animation, animationType, std::nullopt, 0, false, std::nullopt);
  },
  .doDamage = doDamageMessage,
};


extern std::unordered_map<objid, Inventory> scopenameToInventory;
extern GameTypes gametypeSystem;
extern Director director;
extern AiData aiData;
extern bool disableTpsMesh;
extern std::optional<PauseOverride> pauseOverride;


void goToMenu(){
  if (pauseOverride.has_value()){
    pauseOverride.value().menu();
  }else{
    pushHistory({ "mainmenu" }, true);
  }
}

void setPauseMenuOverride(std::optional<std::function<void()>> goToMenuFn){
  if (!goToMenuFn.has_value()){
    pauseOverride = std::nullopt;
  }else{
    pauseOverride = PauseOverride {
      .menu = goToMenuFn.value(),
    };   
  }
}

std::optional<objid> activeSceneForSelected();
void goToLevel(std::string levelShortName, std::optional<std::any> hint, bool forceReload);
void setNoClipMode();
void setFreeCam();
void setNormalMode();
void setEditorMode();

void pauseOnMenu(){
  getGlobalState().userRequestedPause = true;
}
void resumeOnMenu(){
  getGlobalState().userRequestedPause = false;
}

ConsoleInterface consoleInterface  {
      .setNormalMode = setNormalMode,
      .setShowEditor = setEditorMode,
      .setFreeCam = setFreeCam,
      .setNoClip = setNoClipMode,
      .setBackground = setMenuBackground,
      .goToLevel = [](std::optional<std::string> level) -> void {
        modlog("gotolevel", std::string("level loading: ") + level.value());
        goToLevel(level.value(), std::nullopt, false);
      },
      .nextLevel = []() -> void {
        modassert(false, "next level does not exist anymore");
      },
      .takeScreenshot = [](std::string path) -> void {
        gameapi -> saveScreenshot(path);
      },
      .routerPush = [](std::string route, bool replace) -> void {
        pushHistory({ route }, replace);
      },
      .routerPop = []() -> void {
        popHistory();
      },
      .die = []() -> void {
        auto entityId = getEntityForPlayerIndex(getDefaultPlayerIndex()).value();
        killEntity(entityId);
      },
      .toggleKeyboard = []() -> void {
        toggleKeyboard();
      },
      .showWeapon = [](bool showWeapon) -> void {
        modlog("console weapons", std::string("show weapon: ") + print(showWeapon));
        setShowWeaponModel(showWeapon);
      },
      .deliverAmmo = [](int amount) -> void {
        ControlledPlayer& controlledPlayer = getControlledPlayer(getDefaultPlayerIndex());

        if(controlledPlayer.entityId.has_value()){
          deliverEntityAmmo(controlledPlayer.entityId.value(), amount);
        }
      },
      .disableActiveEntity = [](bool enable) -> void {
        disableTpsMesh = enable;
      },
      .spawnByTag = [](std::string tag) -> void {
        spawnFromAllSpawnpoints(director.managedSpawnpoints, tag.c_str());       
      },
      .markLevelComplete = markLevelComplete,
};



UiContext getUiContext(){
  UiContext uiContext {
   .showKeyboard = []() -> bool { 
      return getGlobalState().systemConfig.showKeyboard;
   },
   .playSound = []() -> void {
      playMixedSound(getSymbol("screens/menuclick"), std::nullopt);
    },
  };
  return uiContext;
}