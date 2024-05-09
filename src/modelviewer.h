#ifndef MOD_AFTERWORLD_MODELVIEWER
#define MOD_AFTERWORLD_MODELVIEWER

#include "../../ModEngine/src/cscript/cscript_binding.h"
#include "./util.h"
#include "./global.h"

CScriptBinding modelviewerBinding(CustomApiBindings& api, const char* name);
CScriptBinding particleviewerBinding(CustomApiBindings& api, const char* name);

void emitNewParticleViewerParticle();
void setParticlesViewerShouldEmit(bool shouldEmit);
bool getParticlesViewerShouldEmit();

void setParticleAttribute(std::string, AttributeValue);
std::optional<AttributeValue> getParticleAttribute(std::string);

#endif