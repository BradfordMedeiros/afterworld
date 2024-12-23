#include "./switch.h"

extern CustomApiBindings* gameapi;

void addSwitch(Switches& switches, objid id, std::optional<std::string> onSignal, std::optional<std::string> offSignal){
	switches.switches[id] = ManagedSwitch{
		.switchOn = true,
		.onSignal = onSignal,
		.offSignal = offSignal,
	};

  auto objAttr = getAttrHandle(id);
  auto switchRecording = getStrAttr(objAttr, "switch-recording");
  if (switchRecording.has_value()){
		if (!switches.switches.at(id).switchOn){
      gameapi -> playRecording(id, switchRecording.value(), RECORDING_SETONLY, RecordingOptionResumeAtTime { .elapsedTime = 0.f });
		}else{
			float totalLength = gameapi -> recordingLength(switchRecording.value());
      gameapi -> playRecording(id, switchRecording.value(), RECORDING_SETONLY, RecordingOptionResumeAtTime{ .elapsedTime = totalLength });
		}
  }
}

void removeSwitch(Switches& switches, objid id){
	switches.switches.erase(id);
}

void handleSwitch(Switches& switches, std::string switchValue){ 
  modlog("handle switch", std::string("switch value = ") + switchValue );

  for (auto &[id, switchData] : switches.switches){
	  auto objAttr = getAttrHandle(id);
    auto switchRecording = getStrAttr(objAttr, "switch-recording");
    if (!switchRecording.has_value()){
    	continue;
    }

  	if (!switchData.switchOn && switchData.onSignal.has_value() && switchData.onSignal.value() == switchValue){
  		switchData.switchOn = true;
      gameapi -> playRecording(id, switchRecording.value(), RECORDING_PLAY_ONCE, RecordingOptionResume{});
  	}else if (switchData.switchOn && switchData.offSignal.has_value() && switchData.offSignal.value() == switchValue){
  		switchData.switchOn = false;
      gameapi -> playRecording(id, switchRecording.value(), RECORDING_PLAY_ONCE_REVERSE, RecordingOptionResume{});  // play in reverse
  	}
  }
}