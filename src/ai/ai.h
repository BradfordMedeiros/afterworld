#ifndef MOD_AFTERWORLD_AI
#define MOD_AFTERWORLD_AI

#include "../../../ModEngine/src/cscript/cscript_binding.h"
#include "../global.h"

#include "./worldinfo.h"
#include "./agents/basic_agent.h"
#include "./agents/turret.h"
#include "./agents/tv.h"
#include "./agents/crawler.h"

struct AiData {
  WorldInfo worldInfo;
  std::vector<Agent> agents;
};

AiData createAiData();
void onFrameAi(AiData& aiData, bool showDebug);
void addAiAgent(AiData& aiData, objid id, std::string agentType);
void maybeRemoveAiAgent(AiData& aiData, objid id);

void onObjAdded(AiData& aiData, objid id);
void onObjRemoved(AiData& aiData, objid id);

void onAiOnMessage(AiData& aiData, std::string& key, std::any& value);
DebugConfig debugPrintAi(AiData& aiData);

void maybeReEnableAi(AiData& aiData, objid id);
void maybeDisableAi(AiData& aiData, objid id);

Agent& getAgent(AiData& aiData, objid id);

#endif