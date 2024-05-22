#ifndef MOD_AFTERWORLD_STATE_CONTROLLER
#define MOD_AFTERWORLD_STATE_CONTROLLER

#include <iostream>
#include <vector>
#include "../../ModEngine/src/cscript/cscript_binding.h"
#include "./util.h"

struct ControllerState {
	int fromState;
	int toState;
	int transition;
};
struct ControllerStateAnimation {
	int state;
	std::optional<std::string> animation;
	AnimationType animationBehavior;
};

struct SingleStateController {
	std::vector<ControllerState> transitions;
	std::vector<ControllerStateAnimation> stateToAnimation;
};

struct ControllerEntityState {
	int controller;
	int currentState;
};

struct StateController {
	std::unordered_map<int, SingleStateController> controllers;
	std::unordered_map<objid, ControllerEntityState> entityToState;
};

StateController createStateController();
void addStateController(StateController& controller, std::string controllerName, std::vector<ControllerState> states, std::vector<ControllerStateAnimation> stateAnimations);

void addEntityController(StateController& controller, objid entityId, int controllerId);
void removeEntityController(StateController& controller, objid entityId);
bool hasControllerState(StateController& controller, objid entityId);
bool triggerControllerState(StateController& controller, objid entityId, int transition);
ControllerStateAnimation* stateAnimationForController(StateController& controller, objid entityId);

#endif 