#include "./state-controller.h"

StateController createStateController(){
	return StateController{
		.controllers = {},
		.entityToState = {},
		.pendingAnimations = {},
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
	modlog("statecontroller addEntityController", std::to_string(entityId));
	modassert(controller.entityToState.find(entityId) == controller.entityToState.end(), std::string("state controller - entity already has controller: ") + std::to_string(entityId));
	auto initialEntityState = initialStateForController(controller, controllerId);
	controller.entityToState[entityId] = ControllerEntityState {
		.controller = controllerId,
		.currentState = initialEntityState,
	};
	controller.pendingAnimations.insert(entityId);
}

void removeEntityController(StateController& controller, objid entityId){
	modlog("statecontroller removeEntityController", std::to_string(entityId));
	controller.entityToState.erase(entityId);
}

bool hasControllerState(StateController& controller, objid entityId){
	return controller.entityToState.find(entityId) != controller.entityToState.end();
}

bool triggerControllerState(StateController& controller, objid entityId, int transition){
	modlog("statecontroller animation controller triggerControllerState", nameForSymbol(transition));
	ControllerEntityState& controllerEntityState = controller.entityToState.at(entityId);
	auto controllerStates = controller.controllers.at(controllerEntityState.controller);

	auto entityState = controllerEntityState.currentState;
	for (auto &controllerState : controllerStates.transitions){
		if (controllerState.fromState == entityState && controllerState.transition == transition){
			entityState = controllerState.toState;
			modlog("statecontroller animation controller transitionedState", nameForSymbol(transition) + ", old state = " + nameForSymbol(controllerEntityState.currentState) + ", new state = " + nameForSymbol(controllerState.toState));
			break;
		}
	}

	auto oldState = controllerEntityState.currentState;
	controllerEntityState.currentState = entityState;
	return oldState != controllerEntityState.currentState;
}

ControllerStateAnimation* stateAnimationForController(StateController& controller, objid entityId){
	ControllerEntityState& controllerEntityState = controller.entityToState.at(entityId);
	SingleStateController& controllerStates = controller.controllers.at(controllerEntityState.controller);
	modlog("statecontroller finding for state", nameForSymbol(controllerEntityState.currentState));
	for (auto &stateToAnimation  : controllerStates.stateToAnimation){
		if (stateToAnimation.state == controllerEntityState.currentState){
			return &stateToAnimation;
		}
	}
	return NULL;
}


//relations = relations + getDotInfoForNode(dotInfo.parent.value()) + " -- " + getDotInfoForNode(dotInfo.child) + "\n";


std::optional<std::string> dumpAsString(StateController& controller, std::string name){

  std::string graph = "";
  std::string prefix = "strict digraph {\n";
  std::string suffix = "}"; 

  std::string relations = "";

	for (auto &[id, stateController] : controller.controllers){
		auto controllerName = nameForSymbol(id);
		if (controllerName == name){
			for (auto &controllerTransition : stateController.transitions){
				auto fromState = nameForSymbol(controllerTransition.fromState);
				auto toState = nameForSymbol(controllerTransition.toState);
				auto transition = nameForSymbol(controllerTransition.transition);
				relations += std::string("\"") + fromState + "\"";
				relations += " -> ";
				relations += std::string("\"") + toState + "\"";
				relations += "[label=\"" + transition + "\"]";
				relations += "\n";
			}
		}
	}

  graph = graph + prefix + relations + suffix;

	return graph;
}