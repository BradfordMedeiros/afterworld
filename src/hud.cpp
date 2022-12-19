#include "./hud.h"

extern CustomApiBindings* gameapi;

struct Hud {
	std::vector<objid> managedObjIds;
};

struct HudElement {
	std::string image;
	glm::vec3 position;
	glm::vec3 scale;
};

objid createHudElement(Hud& hud, HudElement& element, objid mainobjId, int index){
  auto sceneId = gameapi -> listSceneId(mainobjId);
  GameobjAttributes attr {
    .stringAttributes = {{ "texture", element.image }, { "layer", "basicui"}, { "mesh", "../gameresources/build/primitives/plane_xy_1x1.gltf" }},
    .numAttributes = {},
    .vecAttr = { .vec3 = {{ "position", element.position }, { "scale", element.scale }}, .vec4 = { {"tint", glm::vec4(1.f, 1.f, 1.f, 1.f) }} },
  };
  std::map<std::string, GameobjAttributes> submodelAttributes;
  return gameapi -> makeObjectAttr(sceneId, "code-hudelement-" + std::to_string(index), attr, submodelAttributes).value();
}

void removeOldHud(Hud& hud){
  for (auto id : hud.managedObjIds){
    gameapi -> removeObjectById(id);
  }
  hud.managedObjIds = {};
}

void changeHud(Hud& hud, std::string name, objid mainobjId){
	removeOldHud(hud);

  auto query = gameapi -> compileSqlQuery(
    "select image, position, scale from huds where name = " + name
  );
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(query, &validSql);

  std::vector<HudElement> hudElements;
  for (auto row : result){
  	hudElements.push_back(HudElement {
  		.image = strFromSqlRow(row, 0),
  		.position = vec3FromSqlRow(row, 1),
  		.scale = vec3FromSqlRow(row, 2),
  	});
  }

  for (int i = 0; i < hudElements.size(); i++){
    HudElement& hudElement = hudElements.at(i);
  	auto id = createHudElement(hud, hudElement, mainobjId, i);
  	hud.managedObjIds.push_back(id);
  }
}


CScriptBinding hudBinding(CustomApiBindings& api, const char* name){
  auto binding = createCScriptBinding(name, api);
  binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
  	Hud* hud = new Hud;
    hud -> managedObjIds = {};
  	changeHud(*hud, "default", id);
  	return hud;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
  	Hud* hud = static_cast<Hud*>(data);
  	delete hud;
  };
  binding.onMessage = [](int32_t id, void* data, std::string& key, AttributeValue& value){
    if (key == "reload-config:hud"){
      Hud* hud = static_cast<Hud*>(data);
      auto strValue = std::get_if<std::string>(&value); 
      modassert(strValue != NULL, "reload-config:hud reload value invalid");
      changeHud(*hud, *strValue, id);
    }
  };


  return binding;
}

