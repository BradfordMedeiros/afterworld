#include "./fps.h"

void renderTraitsPanel(bool includePanel){
  if (includePanel){
    ImGui::Begin("Traits Gameplay");
  }

  if (ImGui::Button("Save")){
      saveTrait("default");
  }

  auto& movementParams = movementParamsByName("default");

  ImGui::SliderFloat("Move Speed", &movementParams.moveSpeed, 0.f, 100.f);
  ImGui::SliderFloat("Move Speed Air", &movementParams.moveSpeedAir, 0.f, 100.f);
  ImGui::SliderFloat("Move Speed Water", &movementParams.moveSpeedWater, 0.f, 100.f);
  ImGui::SliderFloat("Jump Height", &movementParams.jumpHeight, 0.f, 100.f);

  ImGui::SliderFloat("friction", &movementParams.friction, 0.f, 0.1f);
  ImGui::SliderFloat("crouchFriction", &movementParams.crouchFriction, 0.f, 100.f);
  ImGui::SliderFloat("physicsMass", &movementParams.physicsMass, 1.f, 100.f);



  if (includePanel){
    ImGui::End();
  } 
}

void renderWeaponsPanel(bool includePanel){
  if (includePanel){
    ImGui::Begin("Weapons Gameplay");
  }


  auto allWeapons = getWeaponNames();
  std::string selectedWeaponName = selectedWeapon().has_value() ? selectedWeapon().value() : "";

  if (ImGui::BeginCombo("Weapon", selectedWeaponName.c_str())){
      for (int i = 0; i < allWeapons.size(); i++){
          bool selected = false;
          if (ImGui::Selectable(allWeapons.at(i).c_str(), selected)){
             setSelectedWeapon(allWeapons.at(i));
          }
          if (selected){
            ImGui::SetItemDefaultFocus();
          }
      }
      ImGui::EndCombo();
  }

  if (selectedWeaponName != ""){
    auto& gun = getWeaponParamsByGunName(selectedWeaponName);

    if(ImGui::Button("Save")){
        saveWeaponJson(selectedWeaponName);
    }

    ImGui::Checkbox("canHold", &gun.canHold);
    ImGui::Checkbox("isIronsight", &gun.isIronsight);
    ImGui::Checkbox("isRaycast", &gun.isRaycast);
    ImGui::DragFloat("firingRate", &gun.firingRate, 0.01f);

    ImGui::DragFloat("minBloom", &gun.minBloom, 0.01f);
    ImGui::DragFloat("totalBloom", &gun.totalBloom, 0.01f);
    ImGui::DragFloat("bloomLength", &gun.bloomLength, 0.01f);


    ImGui::DragFloat("pos-x", &gun.initialGunPos.x, 0.1f);
    ImGui::DragFloat("pos-y", &gun.initialGunPos.y, 0.1f);
    ImGui::DragFloat("pos-z", &gun.initialGunPos.z, 0.1f);


    ImGui::DragFloat("rot-x", &gun.initialGunRotVec4.x, 0.1f);
    ImGui::DragFloat("rot-y", &gun.initialGunRotVec4.y, 0.1f);
    ImGui::DragFloat("rot-z", &gun.initialGunRotVec4.z, 0.1f);
    ImGui::DragFloat("rot-w", &gun.initialGunRotVec4.w, 0.1f);
    gun.initialGunRot = parseQuat(gun.initialGunRotVec4);    

    ImGui::DragFloat("recoilTranslate-x", &gun.recoilTranslate.x, 0.1f);
    ImGui::DragFloat("recoilTranslate-y", &gun.recoilTranslate.y, 0.1f);
    ImGui::DragFloat("recoilTranslate-z", &gun.recoilTranslate.z, 0.1f);

    ImGui::DragFloat("recoilZoomTranslate-x", &gun.recoilZoomTranslate.x, 0.1f);
    ImGui::DragFloat("recoilZoomTranslate-y", &gun.recoilZoomTranslate.y, 0.1f);
    ImGui::DragFloat("recoilZoomTranslate-z", &gun.recoilZoomTranslate.z, 0.1f);

  
    ImGui::DragFloat("ironsight-x", &gun.ironsightOffset.x, 0.1f);
    ImGui::DragFloat("ironsight-y", &gun.ironsightOffset.y, 0.1f);
    ImGui::DragFloat("ironsight-z", &gun.ironsightOffset.z, 0.1f);


    ImGui::DragFloat("iron-rot-x", &gun.initialIronSightAngle.x, 0.1f);
    ImGui::DragFloat("iron-rot-y", &gun.initialIronSightAngle.y, 0.1f);
    ImGui::DragFloat("iron-rot-z", &gun.initialIronSightAngle.z, 0.1f);
    ImGui::DragFloat("iron-rot-w", &gun.initialIronSightAngle.w, 0.1f);
    gun.ironSightAngle = parseQuat(gun.initialIronSightAngle);    


    ImGui::DragFloat("damage", &gun.damage, 0.1f);
  }

  if (includePanel){
    ImGui::End();
  } 
}

void renderSpawnPanel(bool includePanel){
  if (includePanel){
    ImGui::Begin("Spawn");
  }

/*
     DockButtonConfig {
        .buttonText = "Create Spawnpoint",
        .onClick = []() -> void {
          std::string spawnpointFile("../afterworld/scenes/prefabs/gameplay/spawnpoint.rawscene");
          std::unordered_map<std::string, AttributeValue> attrs;
          attrs["+spawnpoint|spawn"] = std::string("|") + enemyTypes.at(0);
          dockConfigApi.createPrefab(spawnpointFile, attrs);
        },
      },
      DockOptionConfig {
        .options = enemyTypes,
        .onClick = [](std::string&, int index) -> void {
          dockConfigApi.setObjAttr("+spawnpoint|spawn", std::string("|") + enemyTypes.at(index));
        },
        .getSelectedIndex = [](void) -> int {
          auto attr = dockConfigApi.getObjAttr("+spawnpoint|spawn");
          if (!attr.has_value()){
            return -1;
          }
          auto spawnStr = std::get_if<std::string>(&attr.value());
          modassert(spawnStr, "invalid type for spawnStr");

          for (int i = 0; i < enemyTypes.size(); i++){
            if (enemyTypes.at(i) == spawnStr -> substr(1, spawnStr -> size())){
              return i;
            }
          }
          return -1;
        }
      },
      DockCheckboxConfig {
        .label = "Spawn On Load",
        .isChecked = []() -> bool {
          auto attr = dockConfigApi.getObjAttr("+spawnpoint|spawntags");
          if (!attr.has_value()){
            return false;
          }
          auto value = std::get_if<std::string>(&attr.value());
          if (value == NULL){
            return false;
          }
          return *value == "|onload";
        },
        .onChecked = [](bool checked) -> void {
          if (checked){
            dockConfigApi.setObjAttr("+spawnpoint|spawntags", "|onload");
          }else{
            dockConfigApi.setObjAttr("+spawnpoint|spawntags", DeleteAttribute{});
          }
        },
      },
      DockCheckboxConfig {
        .label = "Enable Spawn Tag",
        .isChecked = []() -> bool {
          auto attr = dockConfigApi.getObjAttr("+spawnpoint|spawntags");
          if (!attr.has_value()){
            return false;
          }
          auto strValue = std::get_if<std::string>(&attr.value());
          modassert(strValue, "enable spawn tag wrong type");
          auto values = split(strValue -> substr(1, strValue -> size()), ',');
          for (auto &value : values){
            if (value != "onload"){
              return true;
            }
          }
          return false;
        },
        .onChecked = [](bool checked) -> void {
          if (checked){
            dockConfigApi.setObjAttr("+spawnpoint|spawntags", "|default");
          }else{
            dockConfigApi.setObjAttr("+spawnpoint|spawntags", DeleteAttribute{});
          }
        },
      },
      DockTextboxConfig {
        .label = "Spawn Tag",
        .text = []() -> std::string {
          auto attr = dockConfigApi.getObjAttr("+spawnpoint|spawntags");
          if (!attr.has_value()){
            return "[disabled]";
          }
          auto strValue = std::get_if<std::string>(&attr.value());
          modassert(strValue, "invalid type for spawn tag");
          auto body = strValue -> substr(1, strValue -> size());
          if (body == "onload"){
            return "[disabled]";
          }
          return body; 
        },
        .onEdit = [](std::string value) -> void {
          auto attr = dockConfigApi.getObjAttr("+spawnpoint|spawntags");
          if (!attr.has_value()){
            return;
          }
          auto strValue = std::get_if<std::string>(&attr.value());
          modassert(strValue, "invalid type for spawn tag");
          auto body = strValue -> substr(1, strValue -> size());
          if (body == "onload"){
            return;
          }
          dockConfigApi.setObjAttr("+spawnpoint|spawntags", std::string("|") + value);
        }
      },
      */
  if (includePanel){
    ImGui::End();
  }   
}


void renderPropPanel(bool includePanel, std::optional<objid> sceneId){
  if (includePanel){
    ImGui::Begin("Fps Props");
  }

  if (ImGui::Button("Explosive Barrel")){
    createPrefab(createLocation(), "../afterworld/scenes/prefabs/objects/explosive.rawscene", sceneId.value());
  }


  if (ImGui::Button("Breakable Crate")){
    createPrefab(createLocation(), "../afterworld/scenes/prefabs/objects/crate.rawscene", sceneId.value());
  }

  if (ImGui::Button("Terminal")){
    createPrefab(createLocation(), "../afterworld/scenes/prefabs/objects/terminal.rawscene", sceneId.value());
  }


  if (includePanel){
    ImGui::End();
  }    
}


extern std::optional<UiHealth> uiHealth;
extern AmmoHudInfo ammoInfo;
extern std::optional<std::string> uiWeapon;
extern std::optional<GemCount> uiGemCount;
extern std::optional<glm::vec3> uiVelocity;
extern std::optional<glm::vec2> uiLookVelocity;
extern bool showActivate;
extern std::optional<float> zoomAmount;

void renderFpsHud(bool includePanel){
  if (includePanel){
    ImGui::Begin("Fps Hud");
  }

  ImVec2 available = ImGui::GetContentRegionAvail();
  ImVec2 cursor = ImGui::GetCursorPos();

  ImGui::Dummy(ImVec2(0, 30));

  {
    ImGui::Text("Health: ");
    ImGui::SameLine();
    ImGui::Text(std::to_string(uiHealth.has_value() ? uiHealth.value().health : 0).c_str());
    ImGui::SameLine();
    ImGui::Text(std::to_string(uiHealth.has_value() ? uiHealth.value().totalHealth : 0).c_str());
  }

  {
    ImGui::Text("Ammo: ");
    ImGui::SameLine();
    ImGui::Text(std::to_string(ammoInfo.currentAmmo).c_str());
    ImGui::SameLine();
    ImGui::Text(std::to_string(ammoInfo.totalAmmo).c_str());    
  }

  {
    ImGui::Text("Weapon: ");
    ImGui::SameLine();
    ImGui::Text((uiWeapon.has_value() ? uiWeapon.value() : std::string("unequipped")).c_str());
  }

  {
    ImGui::Text("Gem Count: ");
    ImGui::SameLine();
    ImGui::Text((uiGemCount.has_value() ? (std::to_string(uiGemCount.value().currentCount)) : std::to_string(0)).c_str());
    ImGui::SameLine();
    ImGui::Text((uiGemCount.has_value() ? (std::to_string(uiGemCount.value().totalCount)) : std::to_string(0)).c_str());
  }

  {
    auto velocity = uiVelocity.has_value()? uiVelocity.value() : glm::vec3(0.f, 0.f, 0.f);
    glm::ivec3 speedRounded(velocity.x, velocity.y, velocity.z);
    auto speed = glm::length(velocity);
    ImGui::Text("Speed: ");
    ImGui::SameLine();
    ImGui::Text(print(velocity).c_str());
    ImGui::SameLine();
    ImGui::Text(std::to_string(speed).c_str());
  }

  {
    auto lookVelocity = uiLookVelocity.has_value()? uiLookVelocity.value() : glm::vec2(0.f, 0.f);
    ImGui::Text("Look: ");
    ImGui::SameLine();
    ImGui::Text(print(lookVelocity).c_str());
  }

  ImGui::Text(showActivate ? "press e to activate" : "no activate");
  
  {
    auto zoom = zoomAmount.has_value() ? zoomAmount.value() : 0.f;
    ImGui::Text("Zoom: ");
    ImGui::SameLine();
    ImGui::Text(std::to_string(zoom).c_str());

    if (zoomAmount.has_value()){
      std::string defaultTextureName = paths::UI_ZOOM_IMAGE;
      auto textureId = gameapi->getTextureSamplerId(defaultTextureName).value();
      float imageWidth = 400.f;
      float imageHeight = 400.f;
      ImVec2 imageSize(imageWidth, imageHeight);
      ImGui::SetCursorPos(ImVec2(cursor.x + (available.x - imageSize.x) * 0.5f, cursor.y + (available.y - imageSize.y) * 0.5f));
      ImGui::Image((ImTextureID)(intptr_t)textureId, imageSize, ImVec2(0, 1), ImVec2(1, 0));
    }
  }

  {
    /*  if (imageForHud.has_value()){
      drawTools.drawRect(0.f, 0.f, 2.f, 2.f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), true, std::nullopt, imageForHud.value(), std::nullopt, std::nullopt);
    }
    */
  }

  if (includePanel){
    ImGui::End();
  }     
}