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

bool tryReadCommandResponse(int fd, CommandResponse& command){
    static uint8_t buffer[sizeof(CommandResponse)];
    static size_t received = 0;
    while (received < sizeof(CommandResponse)){
        ssize_t n = read(fd, buffer + received, sizeof(CommandResponse) - received);

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
    std::memcpy(&command, buffer, sizeof(CommandResponse));
    received = 0;
    return true;
}


HardwareState hardwareState {
	.leds = {
		Led { .num = 0, .on = false },
		Led { .num = 1, .on = true  },
	}
};

std::string hardwareStateToString(HardwareState& hardwareState){
    rapidjson::Document doc;
    doc.SetObject();
    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

    rapidjson::Value innerMap(rapidjson::kArrayType);
    for (auto& led : hardwareState.leds){
       rapidjson::Value ledType(rapidjson::kObjectType);
       ledType.AddMember("num", led.num, allocator);
       ledType.AddMember("on", led.on ? true : false, allocator);
       innerMap.PushBack(ledType, allocator);
    }
  
    rapidjson::Value outerKey("leds", allocator);
    doc.AddMember(
        rapidjson::Value(outerKey, allocator),
        innerMap,
        allocator
    );

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);

    return buffer.GetString();
}

void writeHardwareState(){
  auto content = hardwareStateToString(hardwareState);
  std::ofstream file;
  file.open(stateFile);
  file << content;
  file.close();
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



int main(){
	unlink(fifo);
	mkfifo(fifo, 0660);

	unlink(fifoResponse);
	mkfifo(fifoResponse, 0660);

	int readFd = open(fifo, O_RDWR | O_NONBLOCK);
	int responseFd = open(fifoResponse, O_RDWR | O_NONBLOCK);

	std::chrono::seconds(10);


	auto now = std::chrono::steady_clock::now();


	while(true){
		auto currTime = std::chrono::steady_clock::now();
		if (currTime - now > std::chrono::seconds(5)){
			now = currTime;

			Command command {};
			command.type = CommandType::SetLed;

			static bool ledOn = false;
			ledOn = !ledOn;
			command.led = SetLedCommand {
				.led = 0,
				.on = ledOn,
			};
			sendCommand(command);

		}

        static std::optional<pid_t> launchedGame;
		{	// read requests
	 		Command command{};
			if (tryReadCommand(readFd, command)){
    			std::cout << "request = " << print(command) << std::endl;
    			CommandResponse commandResponse{};
    			commandResponse.type = command.type;
    			if(command.type == CommandType::SetLed){
    				for (auto& led : hardwareState.leds){
    					if (led.num == command.led.led){
    						led.on = command.led.on;
    					}
    				}
    			}
                if (command.type == CommandType::StartGame){
                    kill(command.startGame.pid, SIGTERM);
                    system("notify-send 'Tomorrows Bad Arcade' 'Soul Delivery launched from daemon'");
                    launchedGame = launchGameByName("website").value();
                }
    			sendCommandResponse(commandResponse);
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

    	{
    		// read responses
			CommandResponse commandResponse{};
			if (tryReadCommandResponse(responseFd, commandResponse)){
    			std::cout << "response = " << print(commandResponse) << std::endl;

    			writeHardwareState();


				auto hardwareFileStr = readFileContent(stateFile);
				std::cout << "hardwareFileStr: " << hardwareFileStr << std::endl;
			
				bool success = false;
				auto hardwareState = parseHardwareState(hardwareFileStr, &success);
				auto hardwareState2 = hardwareStateToString(hardwareState);
				std::cout << "hardwareFileStr 2: " << hardwareState2 << std::endl;

			}
    	}
	}
	
	unlink(fifo);
	unlink(fifoResponse);
	std::remove(stateFile);

}