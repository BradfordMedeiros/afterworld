#pragma once

#include <iostream>

enum class CommandType : uint8_t {
    SetLed,
};

struct SetLedCommand {
    uint8_t led;
    bool on;
};

struct Command {
    CommandType type;
    union {
        SetLedCommand led;
    };
};

inline void sendLedCommand(Command command){
	std::cout << "this is a mock led command";
}