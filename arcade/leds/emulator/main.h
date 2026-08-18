#pragma once

#include <iostream>

enum class CommandType : uint8_t {
    SetLed,
    StartGame,
};

struct SetLedCommand {
    uint8_t led;
    bool on;
};
struct StartGameCommand {
    uint32_t pid;
};

struct VoidResponse {
    bool isError = false;
};

struct Command {
    CommandType type;
    union {
        SetLedCommand led;
        StartGameCommand startGame;
    };
};

struct CommandResponse {
    CommandType type;
    union {
        VoidResponse led;
        
    };
};


inline std::string print(CommandResponse command){
    std::string data;
    if (command.type == CommandType::SetLed){
        data += std::string("[type = SetLed] ") + (command.led.isError ? "error" : "noerror");
    }
    return data;
}


inline std::string print(Command command){
    std::string data;
    if (command.type == CommandType::SetLed){
        data += std::string("[type = SetLed] ") + std::to_string(command.led.led) + ", on = " + (command.led.on ? "on" : "false") +   ")";
    }
    return data;
}


inline const char* fifo = "/home/brad/gamedev/mosttrusted/afterworld/arcade/leds/emulator/hw-command-request";
inline const char* fifoResponse = "/home/brad/gamedev/mosttrusted/afterworld/arcade/leds/emulator/hw-command-response";
inline const char* stateFile = "/home/brad/gamedev/mosttrusted/afterworld/arcade/leds/emulator/state.json";

inline void sendCommand(Command command){
    int writeFd = open(fifo, O_RDWR | O_NONBLOCK);
    ssize_t written = write(writeFd, &command, sizeof(command));
    if (written != sizeof(command)){
        perror("sendCommand error write");
    }
    close(writeFd);
}

inline void sendCommandResponse(CommandResponse command){
    int writeFd = open(fifoResponse, O_RDWR | O_NONBLOCK);
    ssize_t written = write(writeFd, &command, sizeof(CommandResponse));
    if (written != sizeof(command)){
        perror("sendCommand error write");
    }
    close(writeFd);
}


struct Led {
    int num;
    bool on;
};
struct HardwareState {
    std::vector<Led> leds;
};
inline HardwareState parseHardwareState(std::string content, bool* success){
  rapidjson::Document doc;
  rapidjson::ParseResult ok = doc.Parse(content.c_str());
  if (doc.HasParseError()){
    *success = false;
    return {};
  }

   *success = true;

    HardwareState hardwareState{};

    auto it = doc.FindMember("leds");
    if (it != doc.MemberEnd() && it -> value.IsArray()) {
        const auto& array = it -> value.GetArray();
        for (const auto& item : array) {
            if (item.IsObject()){
                std::optional<Led> led;
                auto ledNum = item.FindMember("num");
                if (ledNum != item.MemberEnd() && ledNum -> value.IsInt()){
                    led = Led {
                        .num = ledNum->value.GetInt(),
                        .on = false,
                    };

                    auto ledOn = item.FindMember("on");
                    if (ledOn != item.MemberEnd() && ledOn -> value.IsBool()){
                         led.value().on = ledOn -> value.GetBool();
                    }
                }
           
                if (led.has_value()){
                    hardwareState.leds.push_back(led.value());
                }
            }
        }
    }

    return hardwareState;
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


inline HardwareState readHardwareState(){
    bool success = false;
    auto fileContent = readFileContent(stateFile);
    return parseHardwareState(fileContent, &success);
}

inline bool isEmulatorConnected(){
  std::ifstream infile(stateFile);
  return infile.good();
}