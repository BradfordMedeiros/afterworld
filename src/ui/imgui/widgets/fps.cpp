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
