#ifndef MOD_AFTERWORLD_MODELVIEWER
#define MOD_AFTERWORLD_MODELVIEWER

#include "../../ModEngine/src/cscript/cscript_binding.h"
#include "./util.h"
#include "./global.h"

void emitNewParticleViewerParticle();
void setParticlesViewerShouldEmit(bool shouldEmit);
bool getParticlesViewerShouldEmit();

void setParticleAttribute(std::string, AttributeValue);
std::optional<AttributeValue> getParticleAttribute(std::string);

#endif