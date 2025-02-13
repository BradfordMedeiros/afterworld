#ifndef MOD_AFTERWORLD_AI
#define MOD_AFTERWORLD_AI

#include "../../../ModEngine/src/cscript/cscript_binding.h"
#include "../global.h"

#include "./worldinfo.h"
#include "./agents/basic_agent.h"
#include "./agents/turret.h"

struct AiData {
  WorldInfo worldInfo;
  std::vector<Agent> agents;
};

AiData createAiData();
void onFrameAi(AiData& aiData);
void addAiAgent(AiData& aiData, objid id, std::string agentType);
void maybeRemoveAiAgent(AiData& aiData, objid id);

void onAiOnMessage(AiData& aiData, std::string& key, std::any& value);
DebugConfig debugPrintAi(AiData& aiData);

void maybeReEnableAi(AiData& aiData, objid id);
void maybeDisableAi(AiData& aiData, objid id);

#endif