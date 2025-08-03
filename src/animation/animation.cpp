#include "./animation.h"

struct StateAndTransition {
	std::string state;
	std::string transition;
};
void addN2Transitions(StateController& controller, std::string name, std::vector<StateAndTransition> states){
	auto nameSymbol = getSymbol(name);
	std::vector<ControllerState>& entitys = controller.controllers.at(nameSymbol).transitions;
	for (int i = 0; i < states.size(); i++){
		for (int j = 0; j < states.size(); j++){
			if (i == j){
				continue;
			}
			auto stateFrom = states.at(i);
			auto stateTo = states.at(j);
			entitys.push_back(ControllerState {
				.fromState = getSymbol(stateFrom.state),
				.toState = getSymbol(stateTo.state),
				.transition = getSymbol(stateTo.transition),
			});
		}
	}
}

void addAnimationController(StateController& controller){
  addStateController(
   	controller, 
   	"character",
   	{},
   	{
   		// basic movement animations
   		ControllerStateAnimation {
   			.state = getSymbol("idle"),
   			.animation = "idle", // make this idle
   			.animationBehavior = LOOP,
   		},
   		ControllerStateAnimation {
   			.state = getSymbol("die"),
   			.animation = "dying",
   			.animationBehavior = FORWARDS,
   		},
   		ControllerStateAnimation {
   			.state = getSymbol("walking"),
   			.animation = "running",
   			.animationBehavior = LOOP,
   		},
   		ControllerStateAnimation {
   			.state = getSymbol("walking-backward"),
   			.animation = "running-backward",
   			.animationBehavior = LOOP,
   		},
   		ControllerStateAnimation {
   			.state = getSymbol("sidestep-right"),
   			.animation = "strafe-right",
   			.animationBehavior = LOOP,
   		},
   		ControllerStateAnimation {
   			.state = getSymbol("sidestep-left"),
   			.animation = "strafe-left",
   			.animationBehavior = LOOP,
   		},

   		/////// misc 
   		ControllerStateAnimation {
   			.state = getSymbol("jump"),
   			.animation = "jump",
   			.animationBehavior = FORWARDS,
   		},
   		
   		// gun animations 
   		ControllerStateAnimation {
   			.state = getSymbol("fire-rifle"),
   			.animation = "rifle-fire",
   			.animationBehavior = LOOP,
   		},
   		
   		ControllerStateAnimation {
   			.state = getSymbol("idle-rifle"),
   			.animation = "rifle-idle", // make this idle
   			.animationBehavior = LOOP,
   		},
   		ControllerStateAnimation {
   			.state = getSymbol("walking-rifle"),
   			.animation = "rifle-run",
   			.animationBehavior = LOOP,
   		},
   		ControllerStateAnimation {
   			.state = getSymbol("walking-rifle-backward"),
   			.animation = "rifle-run-backward",
   			.animationBehavior = LOOP,
   		},
   		ControllerStateAnimation {
   			.state = getSymbol("strafe-right-rifle"),
   			.animation = "rifle-strafe",
   			.animationBehavior = LOOP,
   		},
   		ControllerStateAnimation {
   			.state = getSymbol("strafe-left-rifle"),
   			.animation = "rifle-strafe-mirror",
   			.animationBehavior = LOOP,
   		},
   	}
  );

	addN2Transitions(
		controller, 
		"character", 
		{ 

			StateAndTransition { .state = "idle", .transition = "not-walking" },
			StateAndTransition { .state = "die", .transition = "die" },

			StateAndTransition { .state = "sidestep-right", .transition = "sidestep-right" },
			StateAndTransition { .state = "sidestep-left", .transition = "sidestep-left" },
			StateAndTransition { .state = "walking", .transition = "walking" },
			StateAndTransition { .state = "walking-backward", .transition = "walking-backward" },
			StateAndTransition { .state = "idle-rifle", .transition = "not-walking-rifle" },
			StateAndTransition { .state = "strafe-right-rifle", .transition = "sidestep-right-rifle" },
			StateAndTransition { .state = "strafe-left-rifle", .transition = "sidestep-left-rifle" },
			StateAndTransition { .state = "walking-rifle", .transition = "walking-rifle" },
			StateAndTransition { .state = "walking-rifle-backward", .transition = "walking-rifle-backward" },


			StateAndTransition { .state = "jump", .transition = "jump" },

			//StateAndTransition { .state = "fire-rifle", .transition = "fire-rifle" }, // this requires animations layers
	});


////////////
  addStateController(
   	controller, 
   	"crawler",
   	{},
   	{
   		// basic movement animations
   		ControllerStateAnimation {
   			.state = getSymbol("idle"),
   			.animation = "crawl", // make this idle
   			.animationBehavior = LOOP,
   		},
   		ControllerStateAnimation {
   			.state = getSymbol("die"),
   			.animation = "crawl", // make this idle
   			.animationBehavior = LOOP,
   		},
   		ControllerStateAnimation {
   			.state = getSymbol("walking"),
   			.animation = "crawl",
   			.animationBehavior = LOOP,
   		},
   		ControllerStateAnimation {
   			.state = getSymbol("walking-backward"),
   			.animation = "walking-backward",
   			.animationBehavior = LOOP,
   		},
   		ControllerStateAnimation {
   			.state = getSymbol("sidestep-right"),
   			.animation = "crawl",
   			.animationBehavior = LOOP,
   		},
   		ControllerStateAnimation {
   			.state = getSymbol("sidestep-left"),
   			.animation = "crawl",
   			.animationBehavior = LOOP,
   		},

   		/////// misc 
   		ControllerStateAnimation {
   			.state = getSymbol("jump"),
   			.animation = "jump",
   			.animationBehavior = FORWARDS,
   		},
   		
   		// gun animations 
   		ControllerStateAnimation {
   			.state = getSymbol("fire-rifle"),
   			.animation = "rifle-fire",
   			.animationBehavior = LOOP,
   		},
   		
   		ControllerStateAnimation {
   			.state = getSymbol("idle-rifle"),
   			.animation = "crawl", // make this idle
   			.animationBehavior = LOOP,
   		},
   		ControllerStateAnimation {
   			.state = getSymbol("walking-rifle"),
   			.animation = "crawl",
   			.animationBehavior = LOOP,
   		},
   		
   		ControllerStateAnimation {
   			.state = getSymbol("walking-rifle-backward"),
   			.animation = "crawl",
   			.animationBehavior = LOOP,
   		},
   		ControllerStateAnimation {
   			.state = getSymbol("strafe-right-rifle"),
   			.animation = "crawl",
   			.animationBehavior = LOOP,
   		},
   		ControllerStateAnimation {
   			.state = getSymbol("strafe-left-rifle"),
   			.animation = "crawl",
   			.animationBehavior = LOOP,
   		},
   	}
  );

	addN2Transitions(
		controller, 
		"crawler", 
		{ 
			StateAndTransition { .state = "idle", .transition = "not-walking" },
			StateAndTransition { .state = "die", .transition = "die" },

			StateAndTransition { .state = "sidestep-right", .transition = "sidestep-right" },
			StateAndTransition { .state = "sidestep-left", .transition = "sidestep-left" },
			StateAndTransition { .state = "walking", .transition = "walking" },
			StateAndTransition { .state = "walking-backward", .transition = "walking-backward" },

			StateAndTransition { .state = "idle-rifle", .transition = "not-walking-rifle" },
			StateAndTransition { .state = "strafe-right-rifle", .transition = "sidestep-right-rifle" },
			StateAndTransition { .state = "strafe-left-rifle", .transition = "sidestep-left-rifle" },
			StateAndTransition { .state = "walking-rifle", .transition = "walking-rifle" },
			StateAndTransition { .state = "walking-rifle-backward", .transition = "walking-rifle-backward" },

			StateAndTransition { .state = "jump", .transition = "jump" },

			//StateAndTransition { .state = "fire-rifle", .transition = "fire-rifle" }, // this requires animations layers
	});

}