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
  bool emitParticles;
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

  auto cameraId = gameapi -> getGameObjectByName(">maincamera", gameapi -> listSceneId(id), false).value();

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
    .stringAttributes = {{ "mesh", model }},
    .numAttributes = {},
    .vecAttr = { .vec3 = {}, .vec4 = {}},
  };
  std::map<std::string, GameobjAttributes> submodelAttributes = {};
  modelViewer.viewer.managedObject = gameapi -> makeObjectAttr(
   	gameapi -> listSceneId(id), 
   	std::string("model-viewer-") + uniqueNameSuffix(), 
   	attr, 
   	submodelAttributes
  ).value();
   enforceObjectTransform(modelViewer.viewer, id);
}

void changeObject(ParticleViewerData& particleViewer, objid id){
  modassert(particleViewer.viewer.currentIndex >= 0 && particleViewer.viewer.currentIndex < particleViewer.particleScenes.size(), "invalid index size");
  auto scene = particleViewer.particleScenes.at(particleViewer.viewer.currentIndex);

  if (particleViewer.viewer.managedObject.has_value()){
    gameapi -> removeByGroupId(particleViewer.viewer.managedObject.value());
  }
  GameobjAttributes attr = {
    .stringAttributes = {{ "scene", scene }},
    .numAttributes = {},
    .vecAttr = { .vec3 = {}, .vec4 = {}},
  };
  std::map<std::string, GameobjAttributes> submodelAttributes = {};
  particleViewer.viewer.managedObject = gameapi -> makeObjectAttr(
    gameapi -> listSceneId(id), 
    std::string("[particle-viewer-") + uniqueNameSuffix(), 
    attr, 
    submodelAttributes
  ).value();

  enforceObjectTransform(particleViewer.viewer, id);

}

ViewerData createViewerData(objid id){
  ViewerData viewer {
    .currentIndex = 0,
    .managedObject = std::nullopt,
    .cameraData = {
      .initialCameraPos = glm::vec3(0.f, 0.f, 0.f),
      .rotationXDegrees = 0.f,
      .rotationYDegrees = 0.f,
      .cameraRotationXDegrees = 0.f,
      .scale = 1.f,
      .objectOffset = glm::vec3(0.f, 0.f, 0.f),
    },
  };

  auto cameraId = gameapi -> getGameObjectByName(">maincamera", gameapi -> listSceneId(id), false).value();
  viewer.cameraData.initialCameraPos = gameapi -> getGameObjectPos(cameraId, true);
  return viewer;
}

void handleScroll(ModelViewerData& modelViewer, objid id, double amount){
  modelViewer.viewer.cameraData.scale += amount;
  if (modelViewer.viewer.cameraData.scale > maxScale){
    modelViewer.viewer.cameraData.scale = maxScale;
  }
  if (modelViewer.viewer.cameraData.scale < minScale){
    modelViewer.viewer.cameraData.scale = minScale;
  }
  enforceObjectTransform(modelViewer.viewer, id);
}

void handleMouseRotate(ModelViewerData& modelViewer, objid id, double xPos, double yPos){
  if (leftMouseDown()){
    modelViewer.viewer.cameraData.rotationXDegrees += xPos;
    std::cout << "rotationX: " << modelViewer.viewer.cameraData.rotationXDegrees << std::endl;
    if (modelViewer.viewer.cameraData.rotationXDegrees < 0){
      modelViewer.viewer.cameraData.rotationXDegrees += 360;
    }
    if (modelViewer.viewer.cameraData.rotationXDegrees > 360){
      modelViewer.viewer.cameraData.rotationXDegrees -= 360;
    }
    modelViewer.viewer.cameraData.rotationYDegrees += yPos;
    std::cout << "rotationY: " << modelViewer.viewer.cameraData.rotationYDegrees << std::endl;
    if (modelViewer.viewer.cameraData.rotationYDegrees < 0){
      modelViewer.viewer.cameraData.rotationYDegrees += 360;
    }
    if (modelViewer.viewer.cameraData.rotationYDegrees > 360){
      modelViewer.viewer.cameraData.rotationYDegrees -= 360;
    }
    enforceObjectTransform(modelViewer.viewer, id);
  }
  if (rightMouseDown()){
    modelViewer.viewer.cameraData.cameraRotationXDegrees += xPos;
    std::cout << "cameraRotationXDegrees: " << modelViewer.viewer.cameraData.cameraRotationXDegrees << std::endl;
    if (modelViewer.viewer.cameraData.cameraRotationXDegrees < 0){
      modelViewer.viewer.cameraData.cameraRotationXDegrees += 360;
    }
    if (modelViewer.viewer.cameraData.cameraRotationXDegrees > 360){
      modelViewer.viewer.cameraData.cameraRotationXDegrees -= 360;
    }
    enforceObjectTransform(modelViewer.viewer, id);
  }

  if (middleMouseDown()){
    const float speed = 0.01f;
    modelViewer.viewer.cameraData.objectOffset -= glm::vec3(speed * xPos, speed * yPos, 0.f);
    enforceObjectTransform(modelViewer.viewer, id);
  }
}

void onViewerMessage(ViewerData& viewer, int sizeLimit, std::string& key, std::any& value){
  if (key == "prev-model"){
    viewer.currentIndex--;
  }else if (key == "next-model"){
    viewer.currentIndex++;
  }
  if (viewer.currentIndex < 0){
    viewer.currentIndex = sizeLimit - 1; 
  }
  if (viewer.currentIndex >= sizeLimit){
    viewer.currentIndex = 0;
  }
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
    gameapi -> drawText(std::string("model: ") + modelViewer -> models.at(modelViewer -> viewer.currentIndex), -0.8f, -0.95f, 10.f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt);
  };
  binding.onMouseMoveCallback = [](objid id, void* data, double xPos, double yPos, float xNdc, float yNdc) -> void { 
    ModelViewerData* modelViewer = static_cast<ModelViewerData*>(data);
    handleMouseRotate(*modelViewer, id, xPos, yPos);
  };
  binding.onMessage = [](int32_t id, void* data, std::string& key, std::any& value){
    ModelViewerData* modelViewerData = static_cast<ModelViewerData*>(data);
    ModelViewerData& modelViewer = *modelViewerData;
    onViewerMessage(modelViewer.viewer, modelViewer.models.size(), key, value); 
    changeObject(modelViewer, id);
  };
  binding.onScrollCallback = [](objid id, void* data, double amount) -> void {
    ModelViewerData* modelViewerData = static_cast<ModelViewerData*>(data);
    ModelViewerData& modelViewer = *modelViewerData;
    handleScroll(modelViewer, id, amount);
  };
  return binding;
}



CScriptBinding particleviewerBinding(CustomApiBindings& api, const char* name){
  auto binding = createCScriptBinding(name, api);
  binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    ParticleViewerData* particleViewer = new ParticleViewerData;
    particleViewer -> viewer = createViewerData(id);
    particleViewer -> emitParticles = false;
    particleViewer -> particleScenes = particleScenes;
    changeObject(*particleViewer, id);
    return particleViewer;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
    ParticleViewerData* particleViewer = static_cast<ParticleViewerData*>(data);
    delete particleViewer;
  };
  binding.onFrame = [](int32_t id, void* data) -> void {
    ParticleViewerData* particleViewer = static_cast<ParticleViewerData*>(data);
    gameapi -> drawText(std::string("particles: ") + particleViewer -> particleScenes.at(particleViewer -> viewer.currentIndex), -0.8f, -0.95f, 10.f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt);
  };
  binding.onMouseMoveCallback = [](objid id, void* data, double xPos, double yPos, float xNdc, float yNdc) -> void { 
    //ParticleViewerData* modelViewer = static_cast<ParticleViewerData*>(data);
    //handleMouseRotate(*modelViewer, id, xPos, yPos);
  };

  binding.onMessage = [](int32_t id, void* data, std::string& key, std::any& value){
    ParticleViewerData* particleViewerData = static_cast<ParticleViewerData*>(data);
    onViewerMessage(particleViewerData -> viewer, particleViewerData -> particleScenes.size(), key, value);

    if (key == "modelviewer-emit"){
      auto shouldEmitPtr = anycast<bool>(value); 
      modassert(shouldEmitPtr, "shouldEmitPtr not a bool");
      bool shouldEmit = *shouldEmitPtr;
      modassert(false, std::string("should emit: " ) + print(shouldEmit));
      particleViewerData -> emitParticles = shouldEmit;
    }else if (key == "modelviewer-emit"){
      modassert(false, "emit particle placeholder");
    }

    //changeObject(modelViewer, id);
  };

  binding.onScrollCallback = [](objid id, void* data, double amount) -> void {
    //ParticleViewerData* modelViewerData = static_cast<ModelViewerData*>(data);
    //ParticleViewerData& modelViewer = *modelViewerData;
    //handleScroll(modelViewer, id, amount);
  };
  return binding;
}




