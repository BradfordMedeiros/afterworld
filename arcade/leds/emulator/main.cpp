#include <iostream>
#include <chrono>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <thread>
#include <fcntl.h>
#include <cstring>
#include <vector>
#include <fstream>
#include <sstream>
#include "rapidjson/writer.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include <optional>
#include <signal.h>
#include <sys/wait.h>
#include <filesystem>

#include "./main.h"

bool tryReadCommand(int fd, Command& command){
    static uint8_t buffer[sizeof(Command)];
    static size_t received = 0;
    while (received < sizeof(Command)){
        ssize_t n = read(fd, buffer + received, sizeof(Command) - received);

        if (n > 0){
            received += n;
        }else if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)){
            // Nothing more available right now.
            return false;
        }
        else if (n == 0){
            received = 0;
            return false;
        }else{
            perror("read");
            received = 0;
            return false;
        }
    }
    std::memcpy(&command, buffer, sizeof(Command));
    received = 0;
    return true;
}

std::optional<TmrwGame*> getGameByName(std::vector<TmrwGame>& games, std::string name){
    for (auto& game : games){
        if (game.name == name){
            return &game;
        }
    }
    return std::nullopt;
}

pid_t launchGame(
    const std::string& executable,
    const std::string& workingDirectory,
    const std::vector<std::string>& args = {}) {
    pid_t pid = fork();

    if (pid == -1)
        return -1;

    if (pid == 0) {
        if (chdir(workingDirectory.c_str()) == -1)
            _exit(127);

        std::vector<char*> argv;
        argv.push_back(const_cast<char*>(executable.c_str()));

        for (const auto& arg : args)
            argv.push_back(const_cast<char*>(arg.c_str()));

        argv.push_back(nullptr);

        execv(executable.c_str(), argv.data());

        _exit(127);
    }

    return pid;
}

std::optional<pid_t> launchGameByName(std::string name){
    auto games = listTmrwGames();
    auto tmrwGame = getGameByName(games, name);
    if (tmrwGame.has_value()){
        auto& game = *tmrwGame.value();
        std::cout << "launch game: found: " << name << std::endl;
        return launchGame(game.command, game.workingDir, game.args);
    }

    std::cout << "launch game: not found: " << name << std::endl;
    return std::nullopt;
}


bool hasExited(pid_t pid) {
    int status;
    pid_t result = waitpid(pid, &status, WNOHANG);

    if (result == pid){
        return true;
    }

    return false;
}


ArcadeState* createSharedMemory(){
    shm_unlink("/arcade_state");

    int fd = shm_open("/arcade_state", O_CREAT | O_RDWR, 0600);
    ftruncate(fd, sizeof(ArcadeState));
    void* memory = mmap(nullptr, sizeof(ArcadeState), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);

    ArcadeState* state = static_cast<ArcadeState*>(memory);
    state -> isMuted = false;
    state -> daemonPid = getpid();
    return state;
}


int main(){
    ArcadeState* arcadeState = createSharedMemory();

	unlink(fifo);
	mkfifo(fifo, 0660);

	int readFd = open(fifo, O_RDWR | O_NONBLOCK);
	std::chrono::seconds(10);


	while(true){
        static std::optional<pid_t> launchedGame;
		{	// read requests
	 		Command command{};
			if (tryReadCommand(readFd, command)){
    			std::cout << "request = " << print(command) << std::endl;
    			
                if (command.type == CommandType::StartGame){
                    kill(command.startGame.pid, SIGTERM);
                    system("notify-send 'Tomorrows Bad Arcade' 'Soul Delivery launched from daemon'");
                    launchedGame = launchGameByName("website").value();
                }
                if (command.type == CommandType::SetMuted){
                    arcadeState -> isMuted = command.setMuted.mute;
                }
    		}
    	}
        if (launchedGame.has_value()){
            if (hasExited(launchedGame.value())){
                std::cout << "managed game exited" << std::endl;
                //launchGame("./run.sh", "/home/brad/gamedev/mosttrusted/afterworld", { "-a",  "level=boot"});
                launchGameByName("boot").value();
                launchedGame = std::nullopt;
            }
        }

	}
	
	unlink(fifo);
	std::remove(stateFile);

}