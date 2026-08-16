#ifndef MOD_AFTERWORLD_MODE_BOOT_PLATFORM
#define MOD_AFTERWORLD_MODE_BOOT_PLATFORM

#include "../../../../arcade/leds/emulator/main.h"
namespace platform {
	inline void startGame(){
		system("notify-send 'Tomorrows Bad Arcade' 'Soul Delivery launched'");
		std::this_thread::sleep_for(std::chrono::seconds(5)); // just to simulate launching
		raise(SIGTERM);
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
		sendLedCommand(command);
	}



}


/*
struct ManagedGame {
	std::string name;
    pid_t pid;
};
std::optional<ManagedGame> managedGame;


void launchGame(){
    pid_t pid = fork();
    if (pid == -1) {
        std::cerr << "Failed to fork\n";
        return;
    }
    if (pid == 0) {
        execl("../afterworld/run.sh", "../afterworld/run.sh", (char*)NULL);
        std::cerr << "Failed to execute game\n";
        exit(1);
    }

    managedGame = ManagedGame{
        .name = "Test Game",
        .pid = pid,
    };

    std::thread([pid]() {
        int status = 0;
        pid_t result = waitpid(pid, &status, 0);
        if (result == pid) {
            if (WIFEXITED(status)) {
                std::cout << "Game exited with code " << WEXITSTATUS(status) << std::endl;
            } else if (WIFSIGNALED(status)) {
                std::cout << "Game killed by signal " << WTERMSIG(status) << std::endl;
            }
            if (managedGame && managedGame->pid == pid) {
                managedGame.reset();
            }
        }
    }).detach();
}
*/

#endif 