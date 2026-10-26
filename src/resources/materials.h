#ifndef MOD_AFTERWORLD_MATERIALS
#define MOD_AFTERWORLD_MATERIALS

#include <vector>
#include <string>
#include "../util.h"
#include "./sound.h"
#include "../../../ModEngine/src/scene/serialization.h"

struct ParticleAndEmitter {
  std::string particle;
  objid particleId;
};

struct MaterialToParticle {
  std::string material;
  std::optional<std::string> collisionSound;
  std::optional<std::string> hitParticleClip;
  std::optional<ParticleAndEmitter> hitParticle;  // wherever gun hits, gets parented to the surface
  std::optional<ParticleAndEmitter> splashParticle;  // same as hit particle, but does not get parented.
};

std::optional<std::string> materialTypeForObj(objid id);
std::optional<MaterialToParticle*> getHitMaterial(std::vector<MaterialToParticle>& materials, std::string& materialName);
std::optional<objid> createParticleEmitter(objid sceneId, std::string& particleStr, std::string emitterName);

void loadAllMaterials(objid rootSceneId);
std::vector<MaterialToParticle>& getMaterials();
void onCollisionEnterSound(int32_t sceneId, int32_t obj1, int32_t obj2, glm::vec3 pos);

void loadParticleEmitters(objid rootSceneId);

void emitBlood(objid sceneId, objid lookAtId, glm::vec3 position);
void emitWaterSplash(objid sceneId, objid lookAtId, glm::vec3 position);

void emitKillEffect(glm::vec3 position);
void emitWarp(glm::vec3 position);
void emitSplash(glm::vec3 position);
void emitSparks(glm::vec3 position);
void emitFirework(glm::vec3 position);
void emitFog(glm::vec3 position);

void emitParticle(int symbol, glm::vec3 position, glm::quat rotation = glm::identity<glm::quat>(), glm::vec3 initialVelocity = glm::vec3(0.f, 0.f, 0.f));

#endif

