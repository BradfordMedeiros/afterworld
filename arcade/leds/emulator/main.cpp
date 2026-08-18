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



std::vector<std::string> listAllFiles(std::filesystem::path path) {
  std::vector<std::string> files;
  for(auto &file: std::filesystem::recursive_directory_iterator(path)) {
    if (!std::filesystem::is_directory(file)) {
      files.push_back(file.path());
    }
  }
  return files;
}
std::vector<std::string> split(std::string strToSplit, char delimeter){
  std::vector<std::string> splittedStrings;
  int lowIndex = 0;
  for (int i = 0; i < strToSplit.size(); i++){
    if (strToSplit.at(i) == delimeter){
      auto stringlength = i - lowIndex;
      auto token = strToSplit.substr(lowIndex, stringlength);
      lowIndex = i + 1;
      splittedStrings.push_back(token);
    }
  }
  if (lowIndex != strToSplit.size()){
    auto token = strToSplit.substr(lowIndex, strToSplit.size() - lowIndex);
    splittedStrings.push_back(token);
  }
  if (strToSplit.size() > 0 && strToSplit.at(strToSplit.size() - 1) == delimeter){
    splittedStrings.push_back("");
  }
  return splittedStrings;
}
std::optional<std::string> getExtension(std::string file){
  auto parts = split(file, '.');
  if (parts.size() >= 2){
    return parts.at(parts.size() - 1);  
  }
  return std::nullopt;
}
bool isExtensionType(std::string& file, std::vector<std::string>& extensions){
  bool isValidExtension = false;
  auto extensionData = getExtension(file);
  if (extensionData.has_value()){
    auto extension = extensionData.value();
    for (auto knownExtension : extensions){
      if (extension == knownExtension){
        isValidExtension = true;
        break;
      }
    }
  }
  return isValidExtension;
}

std::vector<std::string> listFilesWithExtensions(std::string folder, std::vector<std::string> extensions){
  std::vector<std::string> files;
  for (auto file : listAllFiles(folder)){
    bool isValidExtension = isExtensionType(file, extensions);
    if (isValidExtension){
      files.push_back(file);
    }
  }
  return files;
}

struct TmrwGame {
    std::string name;
    std::string filepath;
    std::string command;
    std::string workingDir;
    std::vector<std::string> args;
};

std::string print(TmrwGame& game){
    std::string content;
    content += "name = " + game.name + ", ";
    content += "command = " + game.command + ", ";
    content += "workingDir = " + game.workingDir;

    return content;
}

std::optional<TmrwGame> parseGameFile(std::string filepath){
    auto fileContent = readFileContent(filepath);


    std::optional<std::string> name;
    std::optional<std::string> command;
    std::optional<std::string> workingDir;
    std::vector<std::string> args;

    {
      rapidjson::Document doc;
      rapidjson::ParseResult ok = doc.Parse(fileContent.c_str());
      if (doc.HasParseError()){
        std::cout << "error parsing game file: " << filepath << "  (" << fileContent << ")" << std::endl;
        exit(1);
      }


      {
        auto it = doc.FindMember("name");
        if (it != doc.MemberEnd() && it -> value.IsString()) {
          name = it -> value.GetString();
        }        
      }
      {
        auto it = doc.FindMember("command");
        if (it != doc.MemberEnd() && it -> value.IsString()) {
          command = it -> value.GetString();
        }        
      }
      {
        auto it = doc.FindMember("working_directory");
        if (it != doc.MemberEnd() && it -> value.IsString()) {
          workingDir = it -> value.GetString();
        }        
      }
      {
        auto it = doc.FindMember("args");
        if (it != doc.MemberEnd() && it->value.IsArray()) {
            for (const auto& arg : it->value.GetArray()) {
                if (arg.IsString()) {
                    args.emplace_back(arg.GetString());
                }
            }
        }       
      }
    }

    std::cout << fileContent << " ";
    
    bool valid = true;
    if (!name.has_value()){
        std::cout << filepath << " missing name" << std::endl;
        valid = false;
    }
    if (!command.has_value()){
        std::cout << filepath << " missing command" << std::endl;
        valid = false;

    }
    if (!workingDir.has_value()){
        std::cout << filepath << " missing working_directory" << std::endl;
        valid = false;
    }

    if (!valid){
        return std::nullopt;
    }

    return TmrwGame {
        .name = name.value(),
        .filepath = filepath,
        .command = command.value(),
        .workingDir = workingDir.value(),
        .args = args,
    };
}
std::vector<TmrwGame> listGames(){
    std::vector<TmrwGame> games;
    auto allFiles = listFilesWithExtensions("./games", { "tmrw" });
    for (auto & file : allFiles){
        auto game = parseGameFile(file);
        if (game.has_value()){
           games.push_back(game.value()); 
        }
    }
    std::cout << "games: [";
    for (int i = 0; i < games.size(); i++){
        std::cout <<  print(games.at(i)) << " ";
    }
    std::cout << "]" << std::endl;
    return games;
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
    auto games = listGames();
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