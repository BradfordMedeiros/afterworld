#ifndef MOD_AFTERWORLD_AI
#define MOD_AFTERWORLD_AI

#include "../../../ModEngine/src/cscript/cscript_binding.h"
#include "./worldinfo.h"
#include "./agents/basic_agent.h"
#include "./agents/turret.h"
#include "../global.h"

struct AiData {
  WorldInfo worldInfo;
  std::vector<Agent> agents;
};

AiData createAiData();
void onFrameAi(AiData& aiData);
void onAiObjectAdded(AiData& aiData, int32_t idAdded);
void onAiObjectRemoved(AiData& aiData, int32_t idRemoved);
void onAiOnMessage(AiData& aiData, std::string& key, std::any& value);

#endif