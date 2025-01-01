#include "./state-controller.h"

StateController createStateController(){
	return StateController{
		.controllers = {},
		.entityToState = {},
	};
}

void addStateController(StateController& controller, std::string controllerName, std::vector<ControllerState> states, std::vector<ControllerStateAnimation> stateAnimations){
	modassert(controller.controllers.find(getSymbol(controllerName)) == controller.controllers.end(), std::string("state controller - duplicate controller name: ") + controllerName);
	controller.controllers[getSymbol(controllerName)] = SingleStateController {
		.transitions = states,
		.stateToAnimation = stateAnimations,
	};
}

int initialStateForController(StateController& controller, int controllerId){
	return controller.controllers.at(controllerId).transitions.at(0).fromState;
}

void addEntityController(StateController& controller, objid entityId, int controllerId){
	modassert(controller.entityToState.find(entityId) == controller.entityToState.end(), std::string("state controller - entity already has controller: ") + std::to_string(entityId));
	auto initialEntityState = initialStateForController(controller, controllerId);
	controller.entityToState[entityId] = ControllerEntityState {
		.controller = controllerId,
		.currentState = initialEntityState,
	};
}

void removeEntityController(StateController& controller, objid entityId){
	controller.entityToState.erase(entityId);
}

bool hasControllerState(StateController& controller, objid entityId){
	return controller.entityToState.find(entityId) != controller.entityToState.end();
}

bool triggerControllerState(StateController& controller, objid entityId, int transition){
	modlog("animation controller triggerControllerState", nameForSymbol(transition));
	ControllerEntityState& controllerEntityState = controller.entityToState.at(entityId);
	auto controllerStates = controller.controllers.at(controllerEntityState.controller);

	bool transitionedState = false;
	auto entityState = controllerEntityState.currentState;
	for (auto &controllerState : controllerStates.transitions){
		if (controllerState.fromState == entityState && controllerState.transition == transition){
			entityState = controllerState.toState;
			transitionedState = true;
			modlog("animation controller transitionedState", nameForSymbol(transition) + ", new state = " + nameForSymbol(controllerState.toState));
			break;
		}
	}
	controllerEntityState.currentState = entityState;
	return transitionedState;
}

ControllerStateAnimation* stateAnimationForController(StateController& controller, objid entityId){
	ControllerEntityState& controllerEntityState = controller.entityToState.at(entityId);
	SingleStateController& controllerStates = controller.controllers.at(controllerEntityState.controller);
	for (auto &stateToAnimation  : controllerStates.stateToAnimation){
		if (stateToAnimation.state == controllerEntityState.currentState){
			return &stateToAnimation;
		}
	}
	return NULL;
}