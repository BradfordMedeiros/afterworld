#include "./editorui.h"

extern CustomApiBindings* gameapi;

void startMode(bool loadedScene);
void stopMode(bool unloadedScene);

void goToLevel(std::string levelShortName, std::optional<std::any> hint, bool forceReload);
void resetLevel();
void updateArcadeObj(objid id, std::string newType);
void rebootMachine(objid id);
glm::vec3* getColorGrade();
float* getSaturation();
float* getContrast();
glm::vec2* getChromatic();


std::optional<std::string> FileExplorer(std::string directory);

void setSkybox(std::string skybox);
std::vector<std::string> uiListLevelSkyboxes(){
  return { 
    "../gameresources/skybox/space1",
    "./res/textures/skyboxs/desert/",
    "../gameresources/skybox/storm",
  };
}

std::optional<std::string> currentWeather();
void changeWeather(std::optional<std::string> name);




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

    {
      auto colorGrade = getColorGrade();
      float color[3] = {colorGrade -> x, colorGrade -> y, colorGrade -> z};
      if (ImGui::ColorEdit3("Color Grade", color)){
        *colorGrade = glm::vec3(color[0], color[1], color[2]);
      }
    }

    {
      auto saturation = getSaturation();
      ImGui::SliderFloat("Saturation", saturation, -5.f, 2.f);
    }

    {
      auto contrast = getContrast();
      ImGui::SliderFloat("Contrast", contrast, -2.f, 2.f);
    }

    auto chromatic = getChromatic();
    {
      ImGui::SliderFloat("chromatic.x", &chromatic->x, 0.f, 0.5f);
      ImGui::SliderFloat("chromatic.y", &chromatic->y, 0.f, 0.5f);

    }


    auto weather = currentWeather();
    {
      auto weather = currentWeather();
      ImGui::Text("Weather");

      bool noWeatherEnabled = !weather.has_value() || weather.value() == "default";
      bool noWeatherWasEnabled = noWeatherEnabled;
      ImGui::Checkbox("None", &noWeatherEnabled);
      ImGui::SameLine();

      bool rainEnabled = weather.has_value() && weather.value() == "rain";
      bool rainWasEnabled = rainEnabled;
      ImGui::Checkbox("Rain", &rainEnabled);

      if (noWeatherEnabled != noWeatherWasEnabled){
        changeWeather(std::nullopt);
      }else if (rainEnabled != rainWasEnabled){
        changeWeather("rain");
      }
    }

    if(ImGui::Button("Save")){
      /// needs to be moved 
       std::cout << "uiSetLevelInfo here: " << description << std::endl;


  	   updateRawLevelData(selectedLevelStr, UpdateLevel {
  	     .skybox = selectedSkyboxStr,
  	     .description = description, 
  	     .ambient = ambient,
  	     .skyboxColor = skyboxCol,
         .weather = weather,
         .chromatic = *chromatic,
  	   });

    }
  }


  ImGui::Dummy(ImVec2(0, 10));
  

  if (includePanel){
    ImGui::End();
  }     
}

objid makeArcadeObj(objid sceneId){
    std::string name = std::string("arcade-") + uniqueNameSuffix();

    std::unordered_map<std::string, GameobjAttributes> submodelAttributes;
    GameobjAttributes attr { .attr = {
      { "mesh", "../gameresources/build/uncategorized/arcade.gltf" },
      { "tint", glm::vec4(1.f, 1.f, 1.f, 1.f) },
      { "texture", "../ModEngine/res/textures/hexglow.png" },
      { "arcade-cabinet", "true" },

    }};

    GameobjAttributes screenAttr { .attr = {
      { "arcade", "invaders" },
    }};

    submodelAttributes[name + "/screen"] = screenAttr;

    return gameapi -> makeObjectAttr(
      sceneId, 
      name, 
      attr, 
      submodelAttributes
    ).value();
}


void renderArcade(bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId){
  if (includePanel){
    ImGui::Begin("Arcade");
  }

  ImGui::Text("Arcade");

  if (ImGui::Button("Create")){



    makeArcadeObj(sceneId.value());



    /*
    arcade:mesh:../gameresources/build/uncategorized/arcade.gltf
    arcade/screen:arcade:invaders

    arcade:activate:arcade
    arcade:physics:enabled

    arcade:child:>camera
    >camera:position:0 0 1

    arcade/screen:child:!light

    arcade:health:200000*/

  }


  if (objectToDetail.has_value()){
    auto arcade = getSingleAttr(objectToDetail.value(), "arcade-cabinet");
    bool isArcade = arcade.has_value();

    if (isArcade){
      bool isPlayable = false;
      ImGui::Checkbox("Playable", &isPlayable);

      std::vector<std::string> games {
        "none",
        "tennis",
        "invaders",
        "helicopter",
        "rhythm",
        "interact",
      };
      static int selectedGame = 0;
      if (ImGui::BeginCombo("Game", games.at(selectedGame).c_str())){
          for (int i = 0; i < games.size(); i++){
              bool selected = (selectedGame == i);
              if (ImGui::Selectable(games.at(i).c_str(), selected)){
                 selectedGame = i;
              }
              if (selected){
                ImGui::SetItemDefaultFocus();
              }
          }
          ImGui::EndCombo();
      }    

      if (ImGui::Button("Update Arcade")){
        auto screenObj = findChildObjBySuffix(objectToDetail.value(), "screen");
        updateArcadeObj(screenObj.value(), games.at(selectedGame));
      }


    }else{
      ImGui::Text("Not an arcade obj");
    }
  }

  auto selectedFile = FileExplorer("../gameresources/textures/");
  if (selectedFile.has_value()){
    std::cout << "selected file: " << selectedFile.value() << std::endl;
    setGameObjectTexture(objectToDetail.value(), selectedFile.value());
  }

  if (includePanel){
    ImGui::End();
  }     
}


void initImGuiGameUi(){
    registerWidget("Game - Ball", "game", [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
        renderBallGameplay(includePanel);
    });     

    registerWidget("FPS - Weapons", std::nullopt, [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
        renderWeaponsPanel(includePanel);
    });       

    registerWidget("FPS - Traits", std::nullopt, [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
        renderTraitsPanel(includePanel);
    });       

    registerWidget("FPS - Spawn", "game", [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
        renderSpawnPanel(includePanel);
    });     

    registerWidget("FPS - Props", "fps", [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
        renderPropPanel(includePanel, sceneId);
    });     

    registerWidget("Trigger", "game", [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
        renderTriggerPanel(includePanel);
    });   
        
    registerWidget("level", "game", [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
        renderLevelPanel(includePanel);
    });   


    registerWidget("arcade", "game", [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
        renderArcade(includePanel, objectToDetail, sceneId);
    });

    registerWidget("mixing", std::nullopt, [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
        renderMixingPanel(includePanel);
    });

    registerWidget("mixing-detail", std::nullopt, [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
        renderMixingDetailPanel(includePanel);
    });

    registerWidget("game-settings-select", std::nullopt, [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
        renderGameSettingsControlPanel(includePanel);
    });

    registerWidget("game-settings-volume", std::nullopt, [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
        renderGameSettingsView(includePanel);
    });

    registerAction("Start", "Mode", []() -> void {
      startMode(false);
    });
    registerAction("Stop", "Mode", []() -> void {
      stopMode(false);
      resetLevel();
    });


    registerView("Mixing", { "mixing" }, { "mixing-detail" });

    registerView("FPS", { "FPS - Weapons" }, { "FPS - Traits" });


    registerView("GameSettings", { "game-settings-select" }, { "game-settings-volume" });

}



/*
  DockConfiguration {
    .title = "Spawn",
    .configFields = {
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
    }
  },
  */