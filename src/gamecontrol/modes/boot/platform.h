#ifndef MOD_AFTERWORLD_MODE_BOOT_PLATFORM
#define MOD_AFTERWORLD_MODE_BOOT_PLATFORM

#include "../../../../arcade/leds/emulator/main.h"
namespace platform {

	inline ArcadeState* arcadeState = NULL;
	inline void init(){
		arcadeState = openSharedMemory();
		if (arcadeState == NULL){
			std::cout << "could not allocate shared memory" << std::endl;
		}
	}

	inline void startGame(std::string name){
		Command command {};
		command.type = CommandType::StartGame;
		command.startGame = StartGameCommand { 
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

	inline bool isMuted(){
		return arcadeState -> isMuted;
	}
	inline void setMuted(bool shouldMute){
		Command command {};
		command.type = CommandType::SetMuted;
		command.setMuted = SetMutedCommand { 
			.mute = shouldMute,
		};
		sendCommand(command);	
	}

	inline void reboot(){
		system("notify-send 'Tomorrows Bad Arcade' 'Mock Reboot'");
	}


	inline void poll(){
		if (arcadeState == NULL){
			std::cout << "platform.h -> init not called" << std::endl;
			assert(false);
		}
		std::cout << ": " <<  print(*arcadeState) << std::endl;
	}

	static inline bool processExists(pid_t pid){
	    if (kill(pid, 0) == 0){
	        return true;
	    }
	    return errno == EPERM;
	}

	inline bool isConnected(){
		return arcadeState != NULL && processExists(arcadeState -> daemonPid);	
	}

	struct Game {
		std::string name;
		std::string displayName;
	};
	inline std::vector<Game> listGames(){
		auto allGames = listTmrwGames();
		std::vector<Game> games;
		for (auto& game : allGames){
			games.push_back(Game {
				.name = game.filepath,
				.displayName = game.name,
			});
		}
		return games;
	}
}



#endif 