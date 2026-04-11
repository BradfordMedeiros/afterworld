#include "./scene_animation.h"

extern CustomApiBindings* gameapi;
extern StateController animationController;

void pushAlertMessage(std::string message);
std::set<objid>& getDisabledAnimationIds(objid entityId);

bool hasAnimation(objid entityId, std::string& animationName){
  return gameapi -> listAnimations(entityId).count(animationName) > 0;
}

void doAnimationTrigger(objid entityId, const char* transition){
  if (!hasControllerState(animationController, entityId)){
    return;
  }
  bool changedState = triggerControllerState(animationController, entityId, getSymbol(transition));
  if (changedState){
    modlog("statecontroller state changed", std::to_string(entityId));
    animationController.pendingAnimations.insert(entityId);
  }
}

void doStateControllerAnimations(bool validateAnimationControllerAnimations, bool disableAnimation){
  for (auto entityId : animationController.pendingAnimations){
    if (!hasControllerState(animationController, entityId)){
      continue;
    }
    auto stateAnimation = stateAnimationForController(animationController, entityId);
    bool stateAnimationHasAnimation = stateAnimation && stateAnimation -> animation.has_value();
    bool matchingAnimation = stateAnimationHasAnimation && hasAnimation(entityId, stateAnimation -> animation.value());

    if (stateAnimationHasAnimation){
      modlog("statecontroller want animation", stateAnimation -> animation.value());
    }
    if (!disableAnimation && matchingAnimation){
      modlog("statecontroller animation controller play animation for state", nameForSymbol(stateAnimation -> state) + ", " + std::to_string(entityId) + ", " + print(stateAnimation -> animationBehavior));
      pushAlertMessage(nameForSymbol(stateAnimation -> state) + " " + stateAnimation -> animation.value());
      // gameapi -> playAnimation(entityId, stateAnimation -> animation.value(), stateAnimation -> animationBehavior, {});

      gameapi -> playAnimation(entityId, stateAnimation -> animation.value(), stateAnimation -> animationBehavior, getDisabledAnimationIds(entityId), 0, true, std::nullopt);  

    }else{
      if (stateAnimationHasAnimation && !matchingAnimation){
        if (validateAnimationControllerAnimations){
          modassert(false, std::string("no matching animation: ") + stateAnimation -> animation.value());
        }
        modlog("statecontroller animation controller play animation no matching animation for state", nameForSymbol(stateAnimation -> state) + ", for animation: " + stateAnimation -> animation.value() + ", " + std::to_string(entityId));
        pushAlertMessage(nameForSymbol(stateAnimation -> state) + " " + stateAnimation -> animation.value() + " -- missing animation");

      }
      modlog("statecontroller stop animation", std::to_string(entityId));
      gameapi -> stopAnimation(entityId);
    }
  }
  animationController.pendingAnimations = {};
}