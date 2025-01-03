#include "./animation.h"

void addAnimationController(StateController& controller){
  addStateController(
   	controller, 
   	"character",
   	{
  	  ControllerState{
				.fromState = getSymbol("idle"),
				.toState = getSymbol("walking"),
				.transition = getSymbol("walking"),
			},
   		ControllerState{
				.fromState = getSymbol("idle"),
				.toState = getSymbol("sidestep-right"),
				.transition = getSymbol("sidestep-right"),
			},
   		ControllerState{
				.fromState = getSymbol("idle"),
				.toState = getSymbol("sidestep-left"),
				.transition = getSymbol("sidestep-left"),
			},
			ControllerState{
				.fromState = getSymbol("walking"),
				.toState = getSymbol("idle"),
				.transition = getSymbol("not-walking"),
			},
			ControllerState{
				.fromState = getSymbol("walking"),
				.toState = getSymbol("jump"),
				.transition = getSymbol("jump"),
			},
			ControllerState{
				.fromState = getSymbol("sidestep-right"),
				.toState = getSymbol("jump"),
				.transition = getSymbol("jump"),
			},
			ControllerState{
				.fromState = getSymbol("sidestep-left"),
				.toState = getSymbol("jump"),
				.transition = getSymbol("jump"),
			},
			ControllerState{
				.fromState = getSymbol("sidestep-right"),
				.toState = getSymbol("idle"),
				.transition = getSymbol("not-walking"),
			},
			ControllerState{
				.fromState = getSymbol("sidestep-left"),
				.toState = getSymbol("idle"),
				.transition = getSymbol("not-walking"),
			},
			ControllerState{
				.fromState = getSymbol("sidestep-right"),
				.toState = getSymbol("walking"),
				.transition = getSymbol("walking"),
			},
			ControllerState{
				.fromState = getSymbol("sidestep-left"),
				.toState = getSymbol("walking"),
				.transition = getSymbol("walking"),
			},
			ControllerState{
				.fromState = getSymbol("walking"),
				.toState = getSymbol("sidestep-right"),
				.transition = getSymbol("sidestep-right"),
			},
			ControllerState{
				.fromState = getSymbol("walking"),
				.toState = getSymbol("sidestep-left"),
				.transition = getSymbol("sidestep-left"),
			},
   		 ControllerState{
				.fromState = getSymbol("idle"),
				.toState = getSymbol("jump"),
				.transition = getSymbol("jump"),
			},
   		 ControllerState{
				.fromState = getSymbol("jump"),
				.toState = getSymbol("idle"),
				.transition = getSymbol("land"),
			},

			// aiming
   		 ControllerState{
				.fromState = getSymbol("idle"),
				.toState = getSymbol("aiming"),
				.transition = getSymbol("start-aiming"),
			},
   		 ControllerState{
				.fromState = getSymbol("aiming"),
				.toState = getSymbol("idle"),
				.transition = getSymbol("stop-aiming"),
			},

		},
   	{
   		ControllerStateAnimation {
   			.state = getSymbol("idle"),
   			.animation = "idle", // make this idle
   			.animationBehavior = LOOP,
   		},
   		ControllerStateAnimation {
   			.state = getSymbol("walking"),
   			.animation = "walk",
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
   		ControllerStateAnimation {
   			.state = getSymbol("jump"),
   			.animation = "jump",
   			.animationBehavior = FORWARDS,
   		},
   		ControllerStateAnimation {
   			.state = getSymbol("aiming"),
   			.animation = "jump",
   			.animationBehavior = FORWARDS,
   		},
   	}
  );
}