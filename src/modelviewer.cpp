#include "./modelviewer.h"

extern CustomApiBindings* gameapi;

std::optional<objid> managedObject = std::nullopt;
float rotationXDegrees = 0.f;
float rotationYDegrees = 0.f;
glm::vec3 objectOffset(0.f, 0.f, 0.f);
float scale = 1.f;
float minScale = 0.1f;
float maxScale = 5.f;

int currentModelIndex = 0;
std::vector<std::string> models;


void enforceObjectTransform(objid id){
  auto floatX = glm::cos(glm::radians(rotationXDegrees));
  auto floatY = glm::sin(glm::radians(rotationXDegrees));

  auto floatXAroundX = glm::cos(glm::radians(rotationYDegrees));
  auto floatYAroundX = glm::sin(glm::radians(rotationYDegrees));

  auto rotationAroundY = gameapi -> orientationFromPos(glm::vec3(0.f, 0.f, 0.f), glm::vec3(floatX, 0.f, floatY));
  auto rotationAroundX = gameapi -> orientationFromPos(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, floatXAroundX, floatYAroundX));

  //glm::vec3 rotationVec(xPos, yPos, 0.f);
  //auto rotation = gameapi -> orientationFromPos(glm::vec3(0.f, 0.f, 0.f), rotationVec);
  //auto newOrientation = rotation * orientation;
  gameapi -> setGameObjectRot(managedObject.value(), rotationAroundX * rotationAroundY, true);
  gameapi -> setGameObjectScale(managedObject.value(), glm::vec3(scale, scale, scale), true);

  auto position = gameapi -> getGameObjectPos(id, true);
  gameapi -> setGameObjectPosition(managedObject.value(), position + objectOffset, true);

}

void changeObject(objid id, std::string model){
	if (managedObject.has_value()){
		gameapi -> removeByGroupId(managedObject.value());
	}
  GameobjAttributes attr = {
    .stringAttributes = {{ "mesh", model }},
    .numAttributes = {},
    .vecAttr = { .vec3 = {}, .vec4 = {}},
  };
  std::map<std::string, GameobjAttributes> submodelAttributes = {};
   managedObject = gameapi -> makeObjectAttr(
   	gameapi -> listSceneId(id), 
   	std::string("model-viewer-") + uniqueNameSuffix(), 
   	attr, 
   	submodelAttributes
  ).value();

   enforceObjectTransform(id);
}




CScriptBinding modelviewerBinding(CustomApiBindings& api, const char* name){
  auto binding = createCScriptBinding(name, api);
  binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {

  	changeObject(id, "./res/models/box/crate.gltf");
    models = gameapi -> listResources("models");
  	return NULL;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
  	managedObject = std::nullopt;
  };
  binding.onFrame = [](int32_t id, void* data) -> void {
    gameapi -> drawText(std::string("model: ") + models.at(currentModelIndex), -0.8f, -0.95f, 10.f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt);

    std::cout << "left, right " << leftMouseDown() << ", " << rightMouseDown() << std::endl;

  };
  binding.onMouseMoveCallback = [](objid id, void* data, double xPos, double yPos, float xNdc, float yNdc) -> void { 
    if (rightMouseDown() || leftMouseDown()){
      rotationXDegrees += xPos;
      std::cout << "rotationX: " << rotationXDegrees << std::endl;
      if (rotationXDegrees < 0){
        rotationXDegrees += 360;
      }
      if (rotationXDegrees > 360){
        rotationXDegrees -= 360;
      }
      rotationYDegrees += yPos;
      std::cout << "rotationY: " << rotationXDegrees << std::endl;
      if (rotationXDegrees < 0){
        rotationYDegrees += 360;
      }
      if (rotationXDegrees > 360){
        rotationYDegrees -= 360;
      }
      enforceObjectTransform(id);
    }

    if (middleMouseDown()){
      const float speed = 0.01f;
      objectOffset += glm::vec3(speed * xPos, speed * yPos, 0.f);
      enforceObjectTransform(id);
    }
  };

  binding.onMessage = [](int32_t id, void* data, std::string& key, std::any& value){
  	if (!managedObject.has_value()){
  		return;
  	}
		if (key == "prev-model"){
      currentModelIndex--;
 		}else if (key == "next-model"){
      currentModelIndex++;
 		}

    if (currentModelIndex < 0){
      currentModelIndex = models.size() - 1; 
    }
    if (currentModelIndex >= models.size()){
      currentModelIndex = 0;
    }
   
    modlog("modelviewer", std::string("currentModelIndex: ") + std::to_string(currentModelIndex));

    modassert(currentModelIndex >= 0 && currentModelIndex < models.size(), "invalid model index size");
    auto model = models.at(currentModelIndex);
    changeObject(id, model);
  };

  binding.onScrollCallback = [](objid id, void* data, double amount) -> void {
    scale += amount;
    if (scale > maxScale){
      scale = maxScale;
    }
    if (scale < minScale){
      scale = minScale;
    }
    enforceObjectTransform(id);
  };


  return binding;
}





