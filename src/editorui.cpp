#include "./editorui.h"

void startMode(bool loadedScene);
void stopMode(bool unloadedScene);
void reloadVehicleSettings();

void goToLevel(std::string levelShortName, std::optional<std::any> hint, bool forceReload);
void resetLevel();

void setSkybox(std::string skybox);
std::vector<std::string> uiListLevelSkyboxes(){
  return { 
    "../gameresources/skybox/space1",
    "./res/textures/skyboxs/desert/",
    "../gameresources/skybox/storm",
  };
}

void renderBallGameplay(bool includePanel){
  if (includePanel){
    ImGui::Begin("Ball Gameplay");
  }
  static bool doThing = false;

  auto ballConfig = getBallConfig();

  //BallConfig getBallConfig();
  //void setBallConfig(BallConfig ballConfig);

/*
struct BallConfig {
  float magnitude;
  float torque;
  float jumpMagnitude;
  float mass;
  float friction;
  float restitution;
  float gravity;
};*/

  static float speed = 0.f;
  ImGui::DragFloat("magnitude", &ballConfig.magnitude, 0.0f, 200.0f);
  ImGui::DragFloat("torque", &ballConfig.torque, 0.0f, 10.0f);
  ImGui::DragFloat("jump-magnitude", &ballConfig.jumpMagnitude, 0.0f, 10.0f);
  ImGui::DragFloat("mass", &ballConfig.mass, 0.0f, 10.0f);
  ImGui::DragFloat("friction", &ballConfig.friction, 0.0f, 10.0f);
  ImGui::DragFloat("restitution", &ballConfig.restitution, 0.0f, 10.0f);
  ImGui::DragFloat("gravity", &ballConfig.gravity, 0.0f, 10.0f);

  setBallConfig(ballConfig);
  reloadVehicleSettings();

  if (includePanel){
    ImGui::End();
  }
}

void renderMovementPanel(bool includePanel){

  /*
      createSimpleTextboxNumeric("traits", "Speed", "speed", []() -> std::optional<SqlFilter> { return SqlFilter { .column = "profile", .value = "default" }; }),
      createSimpleTextboxNumeric("traits", "Speed Air", "speed-air", []() -> std::optional<SqlFilter> { return SqlFilter { .column = "profile", .value = "default" }; }),
      createSimpleTextboxNumeric("traits", "Jump Height", "jump-height", []() -> std::optional<SqlFilter> { return SqlFilter { .column = "profile", .value = "default" }; }),
      createSimpleTextboxNumeric("traits", "Gravity", "gravity", []() -> std::optional<SqlFilter> { return SqlFilter { .column = "profile", .value = "default" }; }),
      createSimpleTextboxNumeric("traits", "Mass", "mass", []() -> std::optional<SqlFilter> { return SqlFilter { .column = "profile", .value = "default" }; }),
      createSimpleTextboxNumeric("traits", "Friction", "friction", []() -> std::optional<SqlFilter> { return SqlFilter { .column = "profile", .value = "default" }; }),
      createSimpleTextboxNumeric("traits", "Restitution", "restitution", []() -> std::optional<SqlFilter> { return SqlFilter { .column = "profile", .value = "default" }; }),
      createSimpleCheckbox("traits", "Crouch", "crouch", []() -> SqlFilter { return SqlFilter { .column = "profile", .value = "default" }; }),
      createSimpleCheckbox("traits", "Move Vertical", "move-vertical", []() -> SqlFilter { return SqlFilter { .column = "profile", .value = "default" }; }),
  */

  if (includePanel){
    ImGui::Begin("Movement Gameplay");
  }

  static float speed = 0.f;
  ImGui::DragFloat("Speed", &speed, 0.0f, 10.0f);
  ImGui::DragFloat("Speed Air", &speed, 0.0f, 10.0f);
  ImGui::DragFloat("Jump Height", &speed, 0.0f, 10.0f);
  ImGui::DragFloat("Gravity", &speed, 0.0f, 10.0f);
  ImGui::DragFloat("Mass", &speed, 0.0f, 10.0f);
  ImGui::DragFloat("Friction", &speed, 0.0f, 10.0f);
  ImGui::DragFloat("Restitution", &speed, 0.0f, 10.0f);

  bool enabled = false;
  ImGui::Checkbox("Crouch", &enabled);
  ImGui::Checkbox("Move Vertical", &enabled);

  if (includePanel){
    ImGui::End();
  } 
}

void renderWeaponsPanel(bool includePanel){
  if (includePanel){
    ImGui::Begin("Weapons Gameplay");
  }

/*
    .title = "WEAPONS",
    .configFields = {
      DockSelectConfig {
        .selectOptions = SelectOptions {
          .getOptions = []() -> std::vector<std::string>& {
            static std::vector<std::string> options = listGuns();
            return options;
          },
          .toggleExpanded = [](bool expanded) -> void {
            weaponsExpanded = expanded;
          },
          .onSelect = [](int index, std::string& gun) -> void {
            weaponSelectIndex = index;
            weaponsExpanded = false;
            selectedGun = gun;
            modlog("editor gun", std::string("selected gun: ") + gun);
          },
          .currentSelection = []() -> int { return weaponSelectIndex; },
          .isExpanded = []() -> bool { return weaponsExpanded; },
        }
      },
      createSimpleGunCheckbox("Ironsight", "ironsight"),
      createSimpleGunCheckbox("Raycast", "raycast"),
      createSimpleGunCheckbox("Hold", "hold"),
      createSimpleTextboxNumeric("guns","Bloom", "bloom"),
      createSimpleTextboxNumeric("guns","Min Bloom", "minbloom"),
      createSimpleTextboxNumeric("guns","Bloom Length", "bloom-length"),
      createSimpleTextboxNumeric("guns","Horizontal Sway", "bloom-length"),
      createSimpleTextboxNumeric("guns","Vertical Sway", "bloom-length"),
    }
  },
  */

  {
    if (ImGui::Button("Rename")){
      ImGui::OpenPopup("Rename");
    }
  
    if (ImGui::BeginPopupModal("Rename", nullptr, ImGuiWindowFlags_AlwaysAutoResize)){
        std::string name = "";
        ImGui::InputText("Name", &name);
        if (ImGui::Button("OK"))
        {
            std::cout << "create weapon: " << name << std::endl;
    
            ImGui::CloseCurrentPopup();
        }
    
        ImGui::SameLine();
    
        if (ImGui::Button("Cancel"))
        {
            ImGui::CloseCurrentPopup();
        }
    
        ImGui::EndPopup();
    }
  }
  {
    if (ImGui::Button("Delete Weapon")){
      ImGui::OpenPopup("Confirm Delete Weapon");
    }
    if (ImGui::BeginPopupModal("Confirm Delete Weapon", nullptr, ImGuiWindowFlags_AlwaysAutoResize)){
      ImGui::Text("Are you sure?");
      if (ImGui::Button("OK")){
          ImGui::CloseCurrentPopup();
      }
      ImGui::SameLine();
      if (ImGui::Button("Cancel")){
          ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    }
  }

  std::vector<std::string> weapons {
    "weapon_one",
    "weapon_two",
  };

  int selectedWeapon = 0;

  if (ImGui::BeginCombo("Weapon", weapons.at(selectedWeapon).c_str())){
      for (int i = 0; i < weapons.size(); i++){
          bool selected = (selectedWeapon == i);
          if (ImGui::Selectable(weapons.at(i).c_str(), selected)){
             selectedWeapon = i;
          }
          if (selected){
            ImGui::SetItemDefaultFocus();
          }
      }
      ImGui::EndCombo();
  }

  bool enabled = false;

  ImGui::Checkbox("Ironsight", &enabled);
  ImGui::Checkbox("Raycast", &enabled);
  ImGui::Checkbox("Hold", &enabled);

  static float speed = 0.f;
  ImGui::DragFloat("Bloom", &speed, 0.0f, 10.0f);
  ImGui::DragFloat("Min Bloom", &speed, 0.0f, 10.0f);
  ImGui::DragFloat("Bloom Length", &speed, 0.0f, 10.0f);
  ImGui::DragFloat("Horizontal Sway", &speed, 0.0f, 10.0f);
  ImGui::DragFloat("Vertical Sway", &speed, 0.0f, 10.0f);


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


void renderTriggerPanel(bool includePanel){
  if (includePanel){
    ImGui::Begin("Trigger");
  }

/*
      DockButtonConfig {
        .buttonText = "Create Trigger",
        .onClick = []() -> void {
          std::unordered_map<std::string, AttributeValue> attrs;
          std::string triggerFile("../afterworld/scenes/prefabs/gameplay/trigger.rawscene");
          dockConfigApi.createPrefab(triggerFile, attrs);
        },
      },
      DockCheckboxConfig {
        .label = "Oneshot",
        .isChecked = []() -> bool {
          auto value = dockConfigApi.getObjAttr("+trigger|switch-remove");
          if (!value.has_value()){
            return false;
          }
          auto strValue = std::get_if<std::string>(&value.value());
          if (strValue == NULL){
            return false;
          }
          return *strValue == "|enter";
        },
        .onChecked = [](bool checked) -> void {
          if (checked){
            dockConfigApi.setObjAttr("+trigger|switch-remove", "|enter");
          }else{
            dockConfigApi.setObjAttr("+trigger|switch-remove", DeleteAttribute{});
          }
        },
      },
      DockCheckboxConfig {
        .label = "On Enter",
        .isChecked = []() -> bool {
          auto value = dockConfigApi.getObjAttr("+trigger|switch-enter");
          return value.has_value();
        },
        .onChecked = [](bool checked) -> void {
          if (checked){
            dockConfigApi.setObjAttr("+trigger|switch-enter", "|enter");
          }else{
            dockConfigApi.setObjAttr("+trigger|switch-enter", DeleteAttribute{});
          }   
        },
      },
      DockTextboxConfig {
        .label = "On Enter Key",
        .text = []() -> std::string {
          auto value = dockConfigApi.getObjAttr("+trigger|switch-enter");
          if (!value.has_value()){
            return "[disabled]";
          }
          auto attrValue = value.value();
          auto strValue = std::get_if<std::string>(&attrValue);
          modassert(strValue, "invalid type onEnterKey");
          return strValue -> substr(1, strValue -> size());
        },
        .onEdit = [](std::string value) -> void {
          auto enterValue = dockConfigApi.getObjAttr("+trigger|switch-enter");
          if (!enterValue.has_value()){
            return;
          }
          dockConfigApi.setObjAttr("+trigger|switch-enter", std::string("|") + value);
        }
      },
      DockCheckboxConfig {
        .label = "On Exit",
        .isChecked = []() -> bool {
          auto value = dockConfigApi.getObjAttr("+trigger|switch-exit");
          return value.has_value();
        },
        .onChecked = [](bool checked) -> void {
          if (checked){
            dockConfigApi.setObjAttr("+trigger|switch-exit", "|exit");
          }else{
            dockConfigApi.setObjAttr("+trigger|switch-exit", DeleteAttribute{});
          }   
        },
      },
      DockTextboxConfig {
        .label = "On Exit Key",
        .text = []() -> std::string {
          auto value = dockConfigApi.getObjAttr("+trigger|switch-exit");
          if (!value.has_value()){
            return "[disabled]";
          }
          auto attrValue = value.value();
          auto strValue = std::get_if<std::string>(&attrValue);
          modassert(strValue, "invalid type onEnterKey");
          return strValue -> substr(1, strValue -> size());
        },
        .onEdit = [](std::string value) -> void {
          auto enterValue = dockConfigApi.getObjAttr("+trigger|switch-exit");
          if (!enterValue.has_value()){
            return;
          }
          dockConfigApi.setObjAttr("+trigger|switch-exit", std::string("|") + value);
        }
      },
    }
    */
  if (includePanel){
    ImGui::End();
  }   
}


void renderLevelPanel(bool includePanel){
  if (includePanel){
    ImGui::Begin("Levels");
  }

  ImGui::Text("Current Level: None");

  {

    static int selectedLevel = -1;
    auto levels = getRawLevelData();

    std::string selectedLevelStr = "[no level]";
    if (selectedLevel != -1){
      selectedLevelStr = levels.at(selectedLevel).name;
    }
    
    static std::string description = selectedLevel >= 0 ? levels.at(selectedLevel).description : std::string("No description");
    ImGui::InputText("Name", &description);
    
    if (ImGui::BeginCombo("Level", selectedLevelStr.c_str())){
          for (int i = 0; i < levels.size(); i++){
              bool selected = (selectedLevel == i);
              if (ImGui::Selectable(levels.at(i).name.c_str(), selected)){
                selectedLevel = i;
              }
              if (selected){
                ImGui::SetItemDefaultFocus();
              }
          }
          ImGui::EndCombo();
    }
    if(ImGui::Button("Load Level")){
    	goToLevel(selectedLevelStr, std::nullopt, true);
    }

    std::vector<std::string> skyboxs = uiListLevelSkyboxes();
    std::optional<std::string> selectedSkyboxStr;
    {
      static int selectedSkybox = -1;
      if (selectedSkybox != -1){
        selectedSkyboxStr = skyboxs.at(selectedSkybox);
      }
      if (ImGui::BeginCombo("Skybox", selectedSkyboxStr.has_value() ? selectedSkyboxStr.value().c_str() : "none")){
            for (int i = 0; i < skyboxs.size(); i++){
                bool selected = (selectedSkybox == i);
                if (ImGui::Selectable(skyboxs.at(i).c_str(), selected)){
                   selectedSkybox = i;
                }
                if (selected){
                  ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
      }
      if(ImGui::Button("Set skybox")){
        if (selectedSkybox != -1){
        	setSkybox(skyboxs.at(selectedSkybox));
        }
      }
    }

    auto skyboxCol = skyboxColor();
    auto ambient = ambientLight();

    {
      float color[3] = {skyboxCol.r, skyboxCol.g, skyboxCol.b};
      if (ImGui::ColorEdit3("Skybox Color", color)){
        setSkyboxColor(glm::vec3(color[0], color[1], color[2]));
      }
    }

    {
      float color[3] = {ambient.r, ambient.g, ambient.b};
      if (ImGui::ColorEdit3("Ambient", color)){
        setAmbientLightColor(glm::vec3(color[0], color[1], color[2]));
      }
    }

    if(ImGui::Button("Update")){
      /// needs to be moved 
       std::cout << "uiSetLevelInfo here: " << description << std::endl;


  	   updateRawLevelData(selectedLevelStr, UpdateLevel {
  	     .skybox = selectedSkyboxStr,
  	     .description = description, 
  	     .ambient = ambient,
  	     .skyboxColor = skyboxCol,
  	   });

    }
  }


  ImGui::Dummy(ImVec2(0, 10));
  

  if (includePanel){
    ImGui::End();
  }     
}

void initImGuiGameUi(){
	registerWidget("testpanel", "game", [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
  		if (includePanel){
  		  ImGui::Begin("testpanel");
  		}
  		if (includePanel){
  		  ImGui::End();
  		}   
    });

    registerWidget("Game - Ball", "game", [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
        renderBallGameplay(includePanel);
    });     

    registerWidget("FPS - Movement", "game", [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
        renderMovementPanel(includePanel);
    });                 

    registerWidget("FPS - Weapons", "game", [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
        renderWeaponsPanel(includePanel);
    });       

    registerWidget("FPS - Spawn", "game", [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
        renderSpawnPanel(includePanel);
    });     

    registerWidget("Trigger", "game", [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
        renderTriggerPanel(includePanel);
    });   
        
    registerWidget("level", "game", [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
        renderLevelPanel(includePanel);
    });   

    registerAction("Start", "Mode", []() -> void {
      startMode(false);
    });
    registerAction("Stop", "Mode", []() -> void {
      stopMode(false);
      resetLevel();
    });

}


