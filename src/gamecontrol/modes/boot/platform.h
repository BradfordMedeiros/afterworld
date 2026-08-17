#ifndef MOD_AFTERWORLD_MODE_BOOT_PLATFORM
#define MOD_AFTERWORLD_MODE_BOOT_PLATFORM

#include "../../../../arcade/leds/emulator/main.h"
namespace platform {
	inline void startGame(){

		Command command {};
		command.type = CommandType::StartGame;
		command.startGame = StartGameCommand { 
			.pid = getpid(),
		};
		sendCommand(command);

	}

	inline float volume = 1.f;
	inline float getVolume(){
		return volume;
	}
	inline void setVolume(float newVolume){
		volume = newVolume;
	}

	inline bool mute = false;
	inline bool isMuted(){
		return mute;
	}
	inline void setMuted(bool shouldMute){
		mute = shouldMute;
	}

	inline std::string platformInfo(){
		return "Mock Development";
	}

	inline void reboot(){
		system("notify-send 'Tomorrows Bad Arcade' 'Mock Reboot'");
	}

	inline void setLedState(int led, bool on){
		Command command {};
		command.type = CommandType::SetLed;
		command.led = SetLedCommand {
			.led = led,
			.on = on,
		};
		sendCommand(command);
	}

	inline bool isLedStateEnabled(int led){
		auto hwState = readHardwareState();
		for (auto& ledVal : hwState.leds){
			if (ledVal.num == led){
				return ledVal.on;
			}
		}
		return false;
	}

	inline void poll(){
		
	}

	inline bool isConnected(){
		return isEmulatorConnected();
	}

}



#endif 