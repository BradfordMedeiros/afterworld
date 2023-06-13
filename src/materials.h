#ifndef MOD_AFTERWORLD_MATERIALS
#define MOD_AFTERWORLD_MATERIALS

#include <vector>
#include <string>
#include "./util.h"
#include "../../ModEngine/src/scene/serialization.h"

struct ParticleAndEmitter {
  std::string particle;
  objid particleId;
};

struct MaterialToParticle {
  std::string material;
  std::optional<ParticleAndEmitter> hitParticle;  // wherever gun hits, gets parented to the surface
  std::optional<ParticleAndEmitter> splashParticle;  // same as hit particle, but does not get parented. 
};

std::optional<std::string> materialTypeForObj(objid id);
std::optional<MaterialToParticle*> getHitMaterial(std::vector<MaterialToParticle>& materials, std::string& materialName);
GameobjAttributes particleAttributes(std::string& particle);
std::vector<MaterialToParticle> loadMaterials(objid sceneId);

std::optional<objid> createParticleEmitter(objid sceneId, std::string& particleStr, const char* emitterName);

#endif