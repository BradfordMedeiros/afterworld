#include "./hud.h"

extern CustomApiBindings* gameapi;

struct AmmoHudInfo {
  int currentAmmo;
  int totalAmmo;
};

struct Hud {
	std::vector<objid> managedObjIds;

  float health;
  std::optional<AmmoHudInfo> ammoInfo;
};

struct HudElement {
	std::string image;
	glm::vec3 position;
	glm::vec3 scale;
  glm::vec4 tint;
  std::optional<std::string> shader;
};

objid createHudElement(Hud& hud, HudElement& element, objid mainobjId, int index){
  auto sceneId = gameapi -> listSceneId(mainobjId);
  std::map<std::string, std::string> stringAttributes = {{ "texture", element.image }, { "layer", "basicui"}, { "mesh", "../gameresources/build/primitives/plane_xy_1x1.gltf" }};
  if (element.shader.has_value()){
    stringAttributes["shader"] = element.shader.value();
  }
  GameobjAttributes attr {
    .stringAttributes = stringAttributes,
    .numAttributes = {},
    .vecAttr = { .vec3 = {{ "position", element.position }, { "scale", element.scale }}, .vec4 = { {"tint", element.tint }} },
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
    "select image, position, scale, tint, shader from huds where name = " + name,
    {}
  );
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(query, &validSql);

  std::vector<HudElement> hudElements;
  for (auto row : result){
    auto tintStr = strFromSqlRow(row, 3);
    glm::vec4 tint(1.f, 1.f, 1.f, 1.f);
    if (tintStr != ""){
      tint = parseVec4(tintStr);
    }
    auto shaderStr = strFromSqlRow(row, 4);
  	hudElements.push_back(HudElement {
  		.image = strFromSqlRow(row, 0),
  		.position = vec3FromSqlRow(row, 1),
  		.scale = vec3FromSqlRow(row, 2),
      .tint = tint,
      .shader = shaderStr != "" ? std::optional<std::string>(shaderStr) : std::nullopt,
  	});
  }

  for (int i = 0; i < hudElements.size(); i++){
    HudElement& hudElement = hudElements.at(i);
  	auto id = createHudElement(hud, hudElement, mainobjId, i);
  	hud.managedObjIds.push_back(id);
  }
}

void drawbar(float health, float yNdc){
  float width = 0.2f;
  float aspectRatio = 0.1f;
  float widthPercentage = glm::max(0.f, health / 100.f);
  gameapi -> drawRect(-0.8f, yNdc, width + 0.01f, width * aspectRatio + 0.01f, false, glm::vec4(0.2f, 0.2f, 0.2f, 1.f), std::nullopt, true, std::nullopt, std::nullopt);
  gameapi -> drawRect(-0.8f, yNdc, width, width * aspectRatio, false, glm::vec4(0.f, 0.f, 0.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt);
  gameapi -> drawRect(-0.8f, yNdc, widthPercentage * width, width * aspectRatio, false, glm::vec4(0.f, 0.f, 0.8f, 0.6f), std::nullopt, true, std::nullopt, std::nullopt);
}

CScriptBinding hudBinding(CustomApiBindings& api, const char* name){
  auto binding = createCScriptBinding(name, api);
  binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
  	Hud* hud = new Hud;
    hud -> health = 100.f;
    hud -> ammoInfo = std::nullopt;
    hud -> managedObjIds = {};
  	changeHud(*hud, "default", id);
  	return hud;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
  	Hud* hud = static_cast<Hud*>(data);
  	delete hud;
  };
  binding.onMessage = [](int32_t id, void* data, std::string& key, std::any& value){
    Hud* hud = static_cast<Hud*>(data);
    if (key == "reload-config:hud"){
      auto strValue = anycast<std::string>(value); 
      modassert(strValue != NULL, "reload-config:hud reload value invalid");
      changeHud(*hud, *strValue, id);
    }else if (key == "hud-health"){
      auto floatValue = anycast<float>(value);
      modassert(floatValue != NULL, "hud-health value invalid");
      hud -> health = *floatValue;
    }else if (key == "current-gun"){
      auto currentGunMessage = anycast<CurrentGunMessage>(value); 
      modassert(currentGunMessage != NULL, "current-gun value invalid");
      hud -> ammoInfo = AmmoHudInfo {
        .currentAmmo = currentGunMessage -> currentAmmo,
        .totalAmmo = currentGunMessage -> totalAmmo,
      };
    }
  };
  binding.onFrame = [](int32_t id, void* data) -> void {
    if (isPaused()){
      return;
    }
    Hud* hud = static_cast<Hud*>(data);
 
    // healthbar
    gameapi -> drawText("health: " + std::to_string(static_cast<int>(hud -> health)), -0.9, 0.2, 8, false, std::nullopt, std::nullopt, true, std::nullopt, std::nullopt);
    drawbar(hud -> health, 0.1f);
  
    // ammo counter
    if (hud -> ammoInfo.has_value()){
      gameapi -> drawText(std::string("ammo: ") + std::to_string(hud -> ammoInfo.value().currentAmmo) + " / " + std::to_string(hud -> ammoInfo.value().totalAmmo), -0.9, 0.6, 8, false, std::nullopt, std::nullopt, true, std::nullopt, std::nullopt);
    }

  };

  return binding;
}

