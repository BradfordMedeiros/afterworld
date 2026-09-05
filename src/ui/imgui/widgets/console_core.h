#pragma once 

#include <deque>
#include <functional>
#include <iostream>
#include <optional>
#include "../../../util.h"

struct ConsoleInterface {
  std::function<void()> setNormalMode;
  std::function<void()> setShowEditor;
  std::function<void()> setFreeCam;
  std::function<void()> setNoClip;
  std::function<void(std::string)> setBackground;
  std::function<void(std::optional<std::string>)> goToLevel;
  std::function<void()> nextLevel;
  std::function<void(std::string)> takeScreenshot;

  std::function<void(std::string, bool)> routerPush;
  std::function<void()> routerPop;

  std::function<void()> die;
  std::function<void()> toggleKeyboard;
  std::function<void(bool)> showWeapon;
  std::function<void(int)> deliverAmmo;
  std::function<void(bool)> disableActiveEntity;
  std::function<void(std::string)> spawnByTag;
  std::function<void(std::string, float)> markLevelComplete;
};

struct HistoryInstance {
  std::string command;
  bool valid;
};

std::deque<HistoryInstance> loadCommandHistory();

struct CommandDispatch {
  std::string command;
  std::function<std::optional<std::string>(ConsoleInterface&, std::string&, bool*)> fn;
};

extern std::vector<CommandDispatch> commands;
extern std::deque<HistoryInstance> commandHistory;
extern std::deque<HistoryInstance> logHistory;


void initializeConsole();

void executeCommand(ConsoleInterface& consoleInterface, std::string command);