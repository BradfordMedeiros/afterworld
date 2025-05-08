#include "./modelviewer.h"

extern CustomApiBindings* gameapi;

const float minScale = 0.1f;
const float maxScale = 5.f;

struct CameraData {
  glm::vec3 initialCameraPos;
  float rotationXDegrees;
  float rotationYDegrees;
  float cameraRotationXDegrees;
  float scale;
  glm::vec3 objectOffset;

};

struct ViewerData {
  int currentIndex;
  std::optional<objid> managedObject;
  CameraData cameraData;
};

struct ModelViewerData {
  std::vector<std::string> models;
  ViewerData viewer;
};


std::vector<std::string> particleScenes = {
  "../afterworld/scenes/prefabs/particles/blood.rawscene",
  "../afterworld/scenes/prefabs/particles/electricity.rawscene",
  "../afterworld/scenes/prefabs/particles/fire.rawscene",
  "../afterworld/scenes/prefabs/particles/smoke.rawscene",
  "../afterworld/scenes/prefabs/particles/water.rawscene",
};
struct ParticleViewerData {
  std::vector<std::string> particleScenes;
  ViewerData viewer;
};



void enforceObjectTransform(ViewerData& viewer, objid id){
  auto floatX = glm::cos(glm::radians(viewer.cameraData.rotationXDegrees));
  auto floatY = glm::sin(glm::radians(viewer.cameraData.rotationXDegrees));

  auto floatXAroundX = glm::cos(glm::radians(viewer.cameraData.rotationYDegrees));
  auto floatYAroundX = glm::sin(glm::radians(viewer.cameraData.rotationYDegrees));

  auto rotationAroundY = gameapi -> orientationFromPos(glm::vec3(0.f, 0.f, 0.f), glm::vec3(floatX, 0.f, floatY));
  auto rotationAroundX = gameapi -> orientationFromPos(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, floatXAroundX, floatYAroundX));

  //glm::vec3 rotationVec(xPos, yPos, 0.f);
  //auto rotation = gameapi -> orientationFromPos(glm::vec3(0.f, 0.f, 0.f), rotationVec);
  //auto newOrientation = rotation * orientation;
  gameapi -> setGameObjectRot(viewer.managedObject.value(), rotationAroundX * rotationAroundY, true);
  gameapi -> setGameObjectScale(viewer.managedObject.value(), glm::vec3(viewer.cameraData.scale, viewer.cameraData.scale, viewer.cameraData.scale), true);

  auto position = gameapi -> getGameObjectPos(id, true);
  gameapi -> setGameObjectPosition(viewer.managedObject.value(), position, true);

  auto cameraId = gameapi -> getGameObjectByName(">maincamera", gameapi -> listSceneId(id)).value();

  auto newCameraPosition = viewer.cameraData.initialCameraPos ;

  auto distance = glm::distance(viewer.cameraData.initialCameraPos, position) * 2;
  auto cameraAngleOffsetX = distance * glm::cos(glm::radians(viewer.cameraData.cameraRotationXDegrees));
  auto cameraAngleOffsetY = distance * glm::sin(glm::radians(viewer.cameraData.cameraRotationXDegrees));

  std::cout << "camera, angle = " << viewer.cameraData.cameraRotationXDegrees << ", x = " << cameraAngleOffsetX << ", y = " << cameraAngleOffsetY << std::endl;

  auto finalCameraPosition = newCameraPosition + glm::vec3(cameraAngleOffsetX, 0.f, cameraAngleOffsetY) + viewer.cameraData.objectOffset + glm::vec3(0.f, 0.f, -0.5f * distance);
  auto orientation = gameapi -> orientationFromPos(finalCameraPosition, position);
  gameapi -> setGameObjectPosition(cameraId, finalCameraPosition, true); 
  gameapi -> setGameObjectRot(cameraId, orientation, true);

}

void changeObject(ModelViewerData& modelViewer, objid id){
  modassert(modelViewer.viewer.currentIndex >= 0 && modelViewer.viewer.currentIndex < modelViewer.models.size(), "invalid model index size");
  auto model = modelViewer.models.at(modelViewer.viewer.currentIndex);

	if (modelViewer.viewer.managedObject.has_value()){
		gameapi -> removeByGroupId(modelViewer.viewer.managedObject.value());
	}
  GameobjAttributes attr = {
    .attr = {{ "mesh", model }},
  };
  std::unordered_map<std::string, GameobjAttributes> submodelAttributes = {};
  modelViewer.viewer.managedObject = gameapi -> makeObjectAttr(
   	gameapi -> listSceneId(id), 
   	std::string("model-viewer-") + uniqueNameSuffix(), 
   	attr, 
   	submodelAttributes
  ).value();

  modassert(false, "fix this");
  //auto rootId = gameapi -> rootIdForScene(gameapi -> listSceneId(id));
  //gameapi -> makeParent(modelViewer.viewer.managedObject.value(), rootId);

  enforceObjectTransform(modelViewer.viewer, id);
}

void changeObject(ParticleViewerData& particleViewer, objid id){
  modassert(particleViewer.viewer.currentIndex >= 0 && particleViewer.viewer.currentIndex < particleViewer.particleScenes.size(), "invalid index size");
  auto scene = particleViewer.particleScenes.at(particleViewer.viewer.currentIndex);

  if (particleViewer.viewer.managedObject.has_value()){
    gameapi -> removeByGroupId(particleViewer.viewer.managedObject.value());
  }
  GameobjAttributes attr = {
    .attr = {{ "scene", scene }},
  };
  std::unordered_map<std::string, GameobjAttributes> submodelAttributes = {};
  particleViewer.viewer.managedObject = gameapi -> makeObjectAttr(
    gameapi -> listSceneId(id), 
    std::string("[particle-viewer"), 
    attr, 
    submodelAttributes
  ).value();


  // this is only needed b/c bug with adding an object to an existing scene programmatically 
  modassert(false, "todo fix");
  //auto rootId = gameapi -> rootIdForScene(gameapi -> listSceneId(id));  
  //gameapi -> makeParent(particleViewer.viewer.managedObject.value(), rootId);
  //enforceObjectTransform(particleViewer.viewer, id);
}

ViewerData createViewerData(objid id){
  ViewerData viewer {
    .currentIndex = 0,
    .managedObject = std::nullopt,
    .cameraData = {
      .initialCameraPos = glm::vec3(0.f, 0.f, 0.f),
      .rotationXDegrees = 0.f,
      .rotationYDegrees = 0.f,
      .cameraRotationXDegrees = 90.f,
      .scale = 1.f,
      .objectOffset = glm::vec3(0.f, 0.f, 0.f),
    },
  };

  auto cameraId = gameapi -> getGameObjectByName(">maincamera", gameapi -> listSceneId(id)).value();
  viewer.cameraData.initialCameraPos = gameapi -> getGameObjectPos(cameraId, true);
  return viewer;
}

void handleScroll(ViewerData& viewer, objid id, double amount){
  viewer.cameraData.scale += amount;
  if (viewer.cameraData.scale > maxScale){
    viewer.cameraData.scale = maxScale;
  }
  if (viewer.cameraData.scale < minScale){
    viewer.cameraData.scale = minScale;
  }
  enforceObjectTransform(viewer, id);
}

void handleMouseRotate(ViewerData& viewer, objid id, double xPos, double yPos){
  if (leftMouseDown()){
    viewer.cameraData.rotationXDegrees += xPos;
    std::cout << "rotationX: " << viewer.cameraData.rotationXDegrees << std::endl;
    if (viewer.cameraData.rotationXDegrees < 0){
      viewer.cameraData.rotationXDegrees += 360;
    }
    if (viewer.cameraData.rotationXDegrees > 360){
      viewer.cameraData.rotationXDegrees -= 360;
    }
    viewer.cameraData.rotationYDegrees += yPos;
    std::cout << "rotationY: " << viewer.cameraData.rotationYDegrees << std::endl;
    if (viewer.cameraData.rotationYDegrees < 0){
      viewer.cameraData.rotationYDegrees += 360;
    }
    if (viewer.cameraData.rotationYDegrees > 360){
      viewer.cameraData.rotationYDegrees -= 360;
    }
    enforceObjectTransform(viewer, id);
  }
  if (rightMouseDown()){
    viewer.cameraData.cameraRotationXDegrees += xPos;
    std::cout << "cameraRotationXDegrees: " << viewer.cameraData.cameraRotationXDegrees << std::endl;
    if (viewer.cameraData.cameraRotationXDegrees < 0){
      viewer.cameraData.cameraRotationXDegrees += 360;
    }
    if (viewer.cameraData.cameraRotationXDegrees > 360){
      viewer.cameraData.cameraRotationXDegrees -= 360;
    }
    enforceObjectTransform(viewer, id);
  }

  if (middleMouseDown()){
    const float speed = 0.01f;
    viewer.cameraData.objectOffset -= glm::vec3(speed * xPos, speed * yPos, 0.f);
    enforceObjectTransform(viewer, id);
  }
}

void modelViewerNextModel(){
  gameapi -> sendNotifyMessage("next-model", NULL);
}
void modelViewerPrevModel(){
  gameapi -> sendNotifyMessage("prev-model", NULL);
}

bool onViewerMessage(ViewerData& viewer, int sizeLimit, std::string& key, std::any& value){
  bool changedIndex = false;
  if (key == "prev-model"){
    viewer.currentIndex--;
    changedIndex = true;
  }else if (key == "next-model"){
    viewer.currentIndex++;
    changedIndex = true;
  }
  if (viewer.currentIndex < 0){
    viewer.currentIndex = sizeLimit - 1; 
  }
  if (viewer.currentIndex >= sizeLimit){
    viewer.currentIndex = 0;
  }
  return changedIndex;
}

CScriptBinding modelviewerBinding(CustomApiBindings& api, const char* name){
  auto binding = createCScriptBinding(name, api);
  binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    ModelViewerData* modelViewer = new ModelViewerData;
    modelViewer -> viewer = createViewerData(id);
    modelViewer -> models = gameapi -> listResources("models");
    changeObject(*modelViewer, id);
  	return modelViewer;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
    ModelViewerData* modelViewer = static_cast<ModelViewerData*>(data);
    delete modelViewer;
  };
  binding.onFrame = [](int32_t id, void* data) -> void {
    ModelViewerData* modelViewer = static_cast<ModelViewerData*>(data);
    gameapi -> drawText(std::string("model: ") + modelViewer -> models.at(modelViewer -> viewer.currentIndex), -0.8f, -0.95f, 10.f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
  };
  binding.onMouseMoveCallback = [](objid id, void* data, double xPos, double yPos, float xNdc, float yNdc) -> void { 
    ModelViewerData* modelViewer = static_cast<ModelViewerData*>(data);
    handleMouseRotate(modelViewer -> viewer, id, xPos, yPos);
  };
  binding.onMessage = [](int32_t id, void* data, std::string& key, std::any& value){
    ModelViewerData* modelViewerData = static_cast<ModelViewerData*>(data);
    ModelViewerData& modelViewer = *modelViewerData;
    bool changedObject = onViewerMessage(modelViewer.viewer, modelViewer.models.size(), key, value);
    if (changedObject){
      changeObject(modelViewer, id);
    }
  };
  binding.onScrollCallback = [](objid id, void* data, double amount) -> void {
    ModelViewerData* modelViewerData = static_cast<ModelViewerData*>(data);
    handleScroll(modelViewerData -> viewer, id, amount);
  };
  return binding;
}


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

CScriptBinding particleviewerBinding(CustomApiBindings& api, const char* name){
  auto binding = createCScriptBinding(name, api);
  binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    ParticleViewerData* particleViewer = new ParticleViewerData;
    particleViewer -> viewer = createViewerData(id);
    particleViewer -> particleScenes = particleScenes;
    changeObject(*particleViewer, id);
    auto emitterId = getEmitterId(particleViewer -> viewer.managedObject.value());
    if (emitterId.has_value()){
      setGameObjectStateEnabled(emitterId.value(), true);
    }
    return particleViewer;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
    ParticleViewerData* particleViewer = static_cast<ParticleViewerData*>(data);
    delete particleViewer;
  };
  binding.onFrame = [](int32_t id, void* data) -> void {
    ParticleViewerData* particleViewer = static_cast<ParticleViewerData*>(data);
    gameapi -> drawText(std::string("particles: ") + particleViewer -> particleScenes.at(particleViewer -> viewer.currentIndex), -0.8f, -0.95f, 10.f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
  };
  binding.onMouseMoveCallback = [](objid id, void* data, double xPos, double yPos, float xNdc, float yNdc) -> void { 
    ParticleViewerData* particleViewer = static_cast<ParticleViewerData*>(data);
    handleMouseRotate(particleViewer -> viewer, id, xPos, yPos);
  };

  binding.onMessage = [](int32_t id, void* data, std::string& key, std::any& value){
    ParticleViewerData* particleViewerData = static_cast<ParticleViewerData*>(data);
    bool changedObject = onViewerMessage(particleViewerData -> viewer, particleViewerData -> particleScenes.size(), key, value);
    if (changedObject){
      changeObject(*particleViewerData, id);
    }
    if (key == "modelviewer-emit"){
      auto shouldEmitPtr = anycast<bool>(value); 
      modassert(shouldEmitPtr, "shouldEmitPtr not a bool");
      bool shouldEmit = *shouldEmitPtr;
      auto emitterId = getEmitterId(particleViewerData -> viewer.managedObject.value());
      if (emitterId.has_value()){
        setGameObjectStateEnabled(emitterId.value(), shouldEmit);
      }
    }
  };

  binding.onScrollCallback = [](objid id, void* data, double amount) -> void {
    ParticleViewerData* particleViewerData = static_cast<ParticleViewerData*>(data);
    handleScroll(particleViewerData -> viewer, id, amount);
  };
  return binding;
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