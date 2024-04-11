#include "./modelviewer.h"

extern CustomApiBindings* gameapi;

std::optional<objid> managedObject = std::nullopt;
float rotationXDegrees = 0.f;
float rotationYDegrees = 0.f;
glm::vec3 objectOffset(0.f, 0.f, 0.f);
float scale = 1.f;
float minScale = 0.1f;
float maxScale = 5.f;

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

  	return NULL;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
  	managedObject = std::nullopt;
  };
  binding.onFrame = [](int32_t id, void* data) -> void {

  };
  binding.onMouseMoveCallback = [](objid id, void* data, double xPos, double yPos, float xNdc, float yNdc) -> void { 
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

  };

  binding.onKeyCallback = [](int32_t id, void* data, int key, int scancode, int action, int mods) -> void {
    std::cout << "key is: " << key << std::endl;
    if (key == 87){
      objectOffset = objectOffset + glm::vec3(0.f, 0.2f, 0.f);
    }else if (key == 65){
      objectOffset = objectOffset + glm::vec3(-0.2f, 0.f, 0.f);
    }else if (key == 83){
      objectOffset = objectOffset + glm::vec3(0.f, -0.2f, 0.f);
    }else if (key == 68){
      objectOffset = objectOffset + glm::vec3(0.2f, 0.f, 0.f);
    }
    enforceObjectTransform(id);
  };

  binding.onMessage = [](int32_t id, void* data, std::string& key, std::any& value){
  	if (!managedObject.has_value()){
  		return;
  	}
		if (key == "prev-model"){
			changeObject(id, "../gameresources/build/weapons/pistol3.gltf");
 		}else if (key == "next-model"){
 			changeObject(id, "../gameresources/build/characters/plaguerobot.gltf");
 		}  	
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





