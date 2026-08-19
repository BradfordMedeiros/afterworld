#pragma once

#include <iostream>
#include <sys/mman.h>
#include <atomic>

inline const char* fifo = "/home/brad/gamedev/mosttrusted/afterworld/arcade/leds/emulator/hw-command-request";
inline const char* stateFile = "/home/brad/gamedev/mosttrusted/afterworld/arcade/leds/emulator/state.json";
inline const char* gamesDir = "/home/brad/gamedev/mosttrusted/afterworld/arcade/leds/emulator/games";

enum class CommandType : uint8_t {
    StartGame,
    SetMuted,
};

struct StartGameCommand {
    uint32_t pid;
};
struct SetMutedCommand {
    bool mute;
};

struct Command {
    CommandType type;
    union {
        StartGameCommand startGame;
        SetMutedCommand setMuted;
    };
};

inline std::string print(Command command){
    std::string data;
    if (command.type == CommandType::StartGame){
        data += std::string("[type = StartGame] ");
    }
    return data;
}

inline void sendCommand(Command command){
    int writeFd = open(fifo, O_RDWR | O_NONBLOCK);
    ssize_t written = write(writeFd, &command, sizeof(command));
    if (written != sizeof(command)){
        perror("sendCommand error write");
    }
    close(writeFd);
}

inline std::string readFileContent(std::string filepath){
   std::ifstream file(filepath.c_str());
   if (!file.good()){
     throw std::runtime_error("file not found" + filepath);
   }   
   std::stringstream buffer;
   buffer << file.rdbuf();
   return buffer.str();
}


inline std::vector<std::string> listAllFiles(std::filesystem::path path) {
  std::vector<std::string> files;
  for(auto &file: std::filesystem::recursive_directory_iterator(path)) {
    if (!std::filesystem::is_directory(file)) {
      files.push_back(file.path());
    }
  }
  return files;
}
inline std::vector<std::string> split(std::string strToSplit, char delimeter){
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
inline std::optional<std::string> getExtension(std::string file){
  auto parts = split(file, '.');
  if (parts.size() >= 2){
    return parts.at(parts.size() - 1);  
  }
  return std::nullopt;
}
inline bool isExtensionType(std::string& file, std::vector<std::string>& extensions){
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

inline std::vector<std::string> listFilesWithExtensions(std::string folder, std::vector<std::string> extensions){
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

inline std::string print(TmrwGame& game){
    std::string content;
    content += "name = " + game.name + ", ";
    content += "command = " + game.command + ", ";
    content += "workingDir = " + game.workingDir;

    return content;
}

inline std::optional<TmrwGame> parseGameFile(std::string filepath){
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
inline std::vector<TmrwGame> listTmrwGames(){
    std::vector<TmrwGame> games;
    auto allFiles = listFilesWithExtensions(gamesDir, { "tmrw" });
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


struct ArcadeState {
    std::atomic<bool> isMuted;
    std::atomic<int32_t> daemonPid;
};
static_assert(std::atomic<uint32_t>::is_always_lock_free);

inline ArcadeState* openSharedMemory(){
    int fd = shm_open("/arcade_state", O_RDONLY, 0);

    void* memory = mmap(
        nullptr,
        sizeof(ArcadeState),
        PROT_READ,
        MAP_SHARED,
        fd,
        0
    );
    close(fd);

    if (memory == MAP_FAILED) {
        return nullptr;
    }
    ArcadeState* state = static_cast<ArcadeState*>(memory);
    return state;
}


inline std::string print(ArcadeState& arcadeState){
    std::string data;
    data += std::string("isMuted = ") + (arcadeState.isMuted ? "true" : "false");
    return data;
}