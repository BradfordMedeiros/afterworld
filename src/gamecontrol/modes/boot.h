#ifndef MOD_AFTERWORLD_MODE_BOOT
#define MOD_AFTERWORLD_MODE_BOOT

#include "../../global.h"
#include "../gametypes.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

#include <atomic>
#include <iostream>
#include <optional>
#include <string>
#include <thread>

void startBootMode(objid sceneId);
void stopBootMode();

#endif 