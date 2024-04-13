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

struct ModelViewerData {
  std::optional<objid> managedObject;
  std::vector<std::string> models;
  int currentModelIndex;
  CameraData cameraData;
};

std::vector<std::string> particleScenes = {
  "../afterworld/scenes/prefabs/particles/blood.rawscene",
  "../afterworld/scenes/prefabs/particles/electricity.rawscene",
  "../afterworld/scenes/prefabs/particles/fire.rawscene",
  "../afterworld/scenes/prefabs/particles/smoke.rawscene",
  "../afterworld/scenes/prefabs/particles/water.rawscene",
};

void enforceObjectTransform(ModelViewerData& modelViewer, objid id){
  auto floatX = glm::cos(glm::radians(modelViewer.cameraData.rotationXDegrees));
  auto floatY = glm::sin(glm::radians(modelViewer.cameraData.rotationXDegrees));

  auto floatXAroundX = glm::cos(glm::radians(modelViewer.cameraData.rotationYDegrees));
  auto floatYAroundX = glm::sin(glm::radians(modelViewer.cameraData.rotationYDegrees));

  auto rotationAroundY = gameapi -> orientationFromPos(glm::vec3(0.f, 0.f, 0.f), glm::vec3(floatX, 0.f, floatY));
  auto rotationAroundX = gameapi -> orientationFromPos(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, floatXAroundX, floatYAroundX));

  //glm::vec3 rotationVec(xPos, yPos, 0.f);
  //auto rotation = gameapi -> orientationFromPos(glm::vec3(0.f, 0.f, 0.f), rotationVec);
  //auto newOrientation = rotation * orientation;
  gameapi -> setGameObjectRot(modelViewer.managedObject.value(), rotationAroundX * rotationAroundY, true);
  gameapi -> setGameObjectScale(modelViewer.managedObject.value(), glm::vec3(modelViewer.cameraData.scale, modelViewer.cameraData.scale, modelViewer.cameraData.scale), true);

  auto position = gameapi -> getGameObjectPos(id, true);
  gameapi -> setGameObjectPosition(modelViewer.managedObject.value(), position, true);

  auto cameraId = gameapi -> getGameObjectByName(">maincamera", gameapi -> listSceneId(id), false).value();

  auto newCameraPosition = modelViewer.cameraData.initialCameraPos ;

  auto distance = glm::distance(modelViewer.cameraData.initialCameraPos, position) * 2;
  auto cameraAngleOffsetX = distance * glm::cos(glm::radians(modelViewer.cameraData.cameraRotationXDegrees));
  auto cameraAngleOffsetY = distance * glm::sin(glm::radians(modelViewer.cameraData.cameraRotationXDegrees));

  std::cout << "camera, angle = " << modelViewer.cameraData.cameraRotationXDegrees << ", x = " << cameraAngleOffsetX << ", y = " << cameraAngleOffsetY << std::endl;

  auto finalCameraPosition = newCameraPosition + glm::vec3(cameraAngleOffsetX, 0.f, cameraAngleOffsetY) + modelViewer.cameraData.objectOffset + glm::vec3(0.f, 0.f, -0.5f * distance);
  auto orientation = gameapi -> orientationFromPos(finalCameraPosition, position);
  gameapi -> setGameObjectPosition(cameraId, finalCameraPosition, true); 
  gameapi -> setGameObjectRot(cameraId, orientation, true);

}

void changeObject(ModelViewerData& modelViewer, objid id){
  modassert(modelViewer.currentModelIndex >= 0 && modelViewer.currentModelIndex < modelViewer.models.size(), "invalid model index size");
  auto model = modelViewer.models.at(modelViewer.currentModelIndex);

	if (modelViewer.managedObject.has_value()){
		gameapi -> removeByGroupId(modelViewer.managedObject.value());
	}
  GameobjAttributes attr = {
    .stringAttributes = {{ "mesh", model }},
    .numAttributes = {},
    .vecAttr = { .vec3 = {}, .vec4 = {}},
  };
  std::map<std::string, GameobjAttributes> submodelAttributes = {};
   modelViewer.managedObject = gameapi -> makeObjectAttr(
   	gameapi -> listSceneId(id), 
   	std::string("model-viewer-") + uniqueNameSuffix(), 
   	attr, 
   	submodelAttributes
  ).value();

   enforceObjectTransform(modelViewer, id);
}

ModelViewerData createModelViewerData(){
  ModelViewerData modelViewer {
    .managedObject = std::nullopt,
    .models = {},
    .currentModelIndex = 0,
    .cameraData = {
      .initialCameraPos = glm::vec3(0.f, 0.f, 0.f),
      .rotationXDegrees = 0.f,
      .rotationYDegrees = 0.f,
      .cameraRotationXDegrees = 0.f,
      .scale = 1.f,
      .objectOffset = glm::vec3(0.f, 0.f, 0.f),
    },
  };
  return modelViewer;

}

CScriptBinding modelviewerBinding(CustomApiBindings& api, const char* name){
  auto binding = createCScriptBinding(name, api);
  binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    ModelViewerData* modelViewer = new ModelViewerData;
    *modelViewer = createModelViewerData();

    auto cameraId = gameapi -> getGameObjectByName(">maincamera", gameapi -> listSceneId(id), false).value();
    modelViewer -> cameraData.initialCameraPos = gameapi -> getGameObjectPos(cameraId, true);
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
    gameapi -> drawText(std::string("model: ") + modelViewer -> models.at(modelViewer -> currentModelIndex), -0.8f, -0.95f, 10.f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt);
  };
  binding.onMouseMoveCallback = [](objid id, void* data, double xPos, double yPos, float xNdc, float yNdc) -> void { 
    ModelViewerData* modelViewer = static_cast<ModelViewerData*>(data);

    if (leftMouseDown()){
      modelViewer -> cameraData.rotationXDegrees += xPos;
      std::cout << "rotationX: " << modelViewer -> cameraData.rotationXDegrees << std::endl;
      if (modelViewer -> cameraData.rotationXDegrees < 0){
        modelViewer -> cameraData.rotationXDegrees += 360;
      }
      if (modelViewer -> cameraData.rotationXDegrees > 360){
        modelViewer -> cameraData.rotationXDegrees -= 360;
      }
      modelViewer -> cameraData.rotationYDegrees += yPos;
      std::cout << "rotationY: " << modelViewer -> cameraData.rotationYDegrees << std::endl;
      if (modelViewer -> cameraData.rotationYDegrees < 0){
        modelViewer -> cameraData.rotationYDegrees += 360;
      }
      if (modelViewer -> cameraData.rotationYDegrees > 360){
        modelViewer -> cameraData.rotationYDegrees -= 360;
      }
      enforceObjectTransform(*modelViewer, id);
    }

    if (rightMouseDown()){
      modelViewer -> cameraData.cameraRotationXDegrees += xPos;
      std::cout << "cameraRotationXDegrees: " << modelViewer -> cameraData.cameraRotationXDegrees << std::endl;
      if (modelViewer -> cameraData.cameraRotationXDegrees < 0){
        modelViewer -> cameraData.cameraRotationXDegrees += 360;
      }
      if (modelViewer -> cameraData.cameraRotationXDegrees > 360){
        modelViewer -> cameraData.cameraRotationXDegrees -= 360;
      }
      enforceObjectTransform(*modelViewer, id);

    }

    if (middleMouseDown()){
      const float speed = 0.01f;
      modelViewer -> cameraData.objectOffset -= glm::vec3(speed * xPos, speed * yPos, 0.f);
      enforceObjectTransform(*modelViewer, id);
    }
  };

  binding.onMessage = [](int32_t id, void* data, std::string& key, std::any& value){
    ModelViewerData* modelViewerData = static_cast<ModelViewerData*>(data);
    ModelViewerData& modelViewer = *modelViewerData;

  	if (!modelViewer.managedObject.has_value()){
  		return;
  	}
		if (key == "prev-model"){
      modelViewer.currentModelIndex--;
 		}else if (key == "next-model"){
      modelViewer.currentModelIndex++;
 		}else if (key == "modelviewer-emit"){
      auto shouldEmitPtr = anycast<bool>(value); 
      modassert(shouldEmitPtr, "shouldEmitPtr not a bool");
      bool shouldEmit = *shouldEmitPtr;
    
      modassert(false, std::string("should emit: " ) + print(shouldEmit));

    }else if (key == "modelviewer-emit-one"){
      modassert(false, "emit one not yet implemented");
    }
    // modelviewer-emit  , true / false
    // modelviewer-emit-one  , true / false


    if (modelViewer.currentModelIndex < 0){
      modelViewer.currentModelIndex = modelViewer.models.size() - 1; 
    }
    if (modelViewer.currentModelIndex >= modelViewer.models.size()){
      modelViewer.currentModelIndex = 0;
    }
   
    modlog("modelviewer", std::string("currentModelIndex: ") + std::to_string(modelViewer.currentModelIndex));

    changeObject(modelViewer, id);
  };

  binding.onScrollCallback = [](objid id, void* data, double amount) -> void {
    ModelViewerData* modelViewerData = static_cast<ModelViewerData*>(data);
    ModelViewerData& modelViewer = *modelViewerData;

    modelViewer.cameraData.scale += amount;
    if (modelViewer.cameraData.scale > maxScale){
      modelViewer.cameraData.scale = maxScale;
    }
    if (modelViewer.cameraData.scale < minScale){
      modelViewer.cameraData.scale = minScale;
    }
    enforceObjectTransform(modelViewer, id);
  };


  return binding;
}





