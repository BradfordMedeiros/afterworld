#include <iostream>
#include <chrono>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <thread>
#include <fcntl.h>
#include <cstring>
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


std::string print(Command command){
	std::string data;
	if (command.type == CommandType::SetLed){
		data += std::string("[type = SetLed] ( led = ") + std::to_string(command.led.led) + ", on = " + (command.led.on ? "on" : "false") +   ")";
	}
	return data;
}

int main(){
	const char* fifo = "./hw-command";

	unlink(fifo);
	mkfifo(fifo, 0660);
	

	int fd = open(fifo, O_RDWR | O_NONBLOCK);

	std::chrono::seconds(10);


	auto now = std::chrono::steady_clock::now();

	while(true){
		auto currTime = std::chrono::steady_clock::now();
		if (currTime - now > std::chrono::seconds(5)){
			now = currTime;
			std::cout << "writing command" << std::endl;

			Command command {};
			command.type = CommandType::SetLed;

			static bool ledOn = false;
			ledOn = !ledOn;
			command.led = SetLedCommand {
				.led = 0,
				.on = ledOn,
			};
			//writeCommand(command);

			std::cout << "writing command" << std::endl;
			ssize_t written = write(fd, &command, sizeof(command));
			std::cout << "finished writing" << std::endl;

			if (written != sizeof(command)){
    			perror("write");
			}
		}

	 	Command command;

		if (tryReadCommand(fd, command)){
    		std::cout << print(command) << std::endl;
    		std::cout << "read success" << std::endl;
    	}else{
    	}
	}
//
	sleep(5);


	unlink("./hw-command");

}