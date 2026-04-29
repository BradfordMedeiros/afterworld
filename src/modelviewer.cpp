#include "./modelviewer.h"

extern CustomApiBindings* gameapi;

std::optional<objid> getEmitterId(objid rootObjId){
  return std::nullopt;
  //modlog("emitter child scenes of: ", gameapi -> getGameObjNameForId(rootObjId).value());
  //auto prefabScenes = gameapi -> childScenes(gameapi -> listSceneId(rootObjId));
  //modlog("emitter", std::string("checking num scenes: " ) + std::to_string(prefabScenes.size()));
  //std::optional<objid> emitterId;
  //for (auto &prefabSceneId : prefabScenes){
  //  modassert(!emitterId.has_value(), "multiple emitters across the scenes");
  //  modlog("emitter", std::string("checking scene: ") + std::to_string(prefabSceneId));
  //  emitterId = gameapi -> getGameObjectByName("+particles", prefabSceneId, false).value();
  //}
  //return emitterId;
}

std::optional<objid> getEmitter(){
  auto emitterPrefab = findObjByShortName("[particle-viewer", std::nullopt);
  if (!emitterPrefab.has_value()){
    return std::nullopt;
  }

  auto emitterId = getEmitterId(emitterPrefab.value());
  return emitterId;
}


bool shouldEmitParticleViewerParticles = true;
void emitNewParticleViewerParticle(){
  auto emitterId = getEmitter();
  if (emitterId.has_value()){
    gameapi -> emit(emitterId.value(), std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
  }
}


void setParticlesViewerShouldEmit(bool shouldEmit){
  auto emitterId = getEmitter();
  if (emitterId.has_value()){
    gameapi -> setSingleGameObjectAttr(emitterId.value(), "state", shouldEmit ? std::string("enabled") : std::string("disabled"));
    shouldEmitParticleViewerParticles = shouldEmit;    
  }
}

bool getParticlesViewerShouldEmit(){
  return shouldEmitParticleViewerParticles;
}

void setParticleAttribute(std::string field, AttributeValue value){
  auto emitterId = getEmitter();
  if (emitterId.has_value()){
    gameapi -> setSingleGameObjectAttr(emitterId.value(), field.c_str(), value);
  }
}
std::optional<AttributeValue> getParticleAttribute(std::string field){
  auto emitterId = getEmitter();
  if (!emitterId.has_value()){
    return std::nullopt;
  }

  return getObjectAttribute(emitterId.value(), field.c_str());
}