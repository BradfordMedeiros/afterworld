#include "./editorui.h"

extern CustomApiBindings* gameapi;

void startMode(bool loadedScene);
void stopMode(bool unloadedScene);
void reloadVehicleSettings();

void goToLevel(std::string levelShortName, std::optional<std::any> hint, bool forceReload);
void resetLevel();
void updateArcadeObj(objid id, std::string newType);
void rebootMachine(objid id);

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

void renderBallGameplay(bool includePanel){
  if (includePanel){
    ImGui::Begin("Ball Gameplay");
  }

  if (ImGui::Button("Save")){
    saveBallConfig();
  }

  auto& ballConfig = getBallConfig();

  ImGui::DragFloat("magnitude", &ballConfig.magnitude, 0.0f, 200.0f);
  ImGui::DragFloat("torque", &ballConfig.torque, 0.0f, 10.0f);
  ImGui::DragFloat("jump-magnitude", &ballConfig.jumpMagnitude, 0.0f, 10.0f);
  ImGui::DragFloat("mass", &ballConfig.mass, 0.0f, 10.0f);
  ImGui::DragFloat("friction", &ballConfig.friction, 0.0f, 10.0f);
  ImGui::DragFloat("restitution", &ballConfig.restitution, 0.0f, 10.0f);
  ImGui::DragFloat("gravity", &ballConfig.gravity, 0.0f, 10.0f);

  reloadVehicleSettings();

  if (includePanel){
    ImGui::End();
  }
}


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

/*


  int totalAmmo = 0;

  // model specific
  float recoilLength = 0.f;
  float recoilPitchRadians = 0.f;
  glm::vec3 recoilZoomTranslate = glm::vec3(0.f, 0.f, 0.f);

  std::optional<std::string> fireAnimation;
  std::optional<std::string> idleAnimation;
  glm::quat initialGunRot = glm::identity<glm::quat>();
  glm::vec4 initialGunRotVec4 = glm::vec4(0.f, 0.f, 0.f, 0.f);
  glm::quat ironSightAngle = glm::identity<glm::quat>();

  glm::vec3 scale = glm::vec3(1.f, 1.f, 1.f);
  std::string soundpath;
  std::string modelpath;

  std::string muzzleParticleStr;
  std::string hitParticleStr;
  std::string projectileParticleStr;

  float damage = 0.f;
  */

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

    if(ImGui::Button("Update")){
      /// needs to be moved 
       std::cout << "uiSetLevelInfo here: " << description << std::endl;


  	   updateRawLevelData(selectedLevelStr, UpdateLevel {
  	     .skybox = selectedSkyboxStr,
  	     .description = description, 
  	     .ambient = ambient,
  	     .skyboxColor = skyboxCol,
         .weather = weather,
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


struct SoundFolderNode{
    std::string name;
    std::vector<int> sounds;
    std::vector<SoundFolderNode> children;
};
SoundFolderNode BuildSoundTree(std::vector<SoundBinding>& soundBindings){
    SoundFolderNode root;
    for (int i = 0; i < soundBindings.size(); i++){
        auto& binding = soundBindings.at(i);
        SoundFolderNode* node = &root;
        for (auto& folder : binding.folder){
            auto it = std::find_if(
                node->children.begin(),
                node->children.end(),
                [&](const SoundFolderNode& child)
                {
                    return child.name == folder;
                });

            if (it == node->children.end()){
                node->children.push_back(SoundFolderNode{
                    .name = folder
                });
                node = &node->children.back();
            }else{
                node = &*it;
            }
        }
        node->sounds.push_back(i);
    }
    return root;
}

void DrawSoundTree(
    const SoundFolderNode& node,
    const std::vector<SoundBinding>& soundBindings,
    int& selectedSound){
    for (const auto& child : node.children){
        if (ImGui::TreeNode(child.name.c_str())){
            DrawSoundTree(child, soundBindings, selectedSound);
            ImGui::TreePop();
        }
    }

    for (int index : node.sounds){
        const auto& sound = soundBindings[index];
        if (ImGui::Selectable(sound.sound.c_str(), selectedSound == index)){
            selectedSound = index;
        }
    }
}



void renderMixingPanel(bool includePanel){
  if (includePanel){
    ImGui::Begin("Mixing");
  }

  auto soundBindings = getSoundInfo().soundBindings;
  SoundFolderNode soundTree = BuildSoundTree(soundBindings);

  int selectedSound = -1;
  DrawSoundTree(soundTree, soundBindings, selectedSound);
  if (selectedSound != -1){
    std::cout << "selected sound: " << selectedSound <<  " " << soundBindings.at(selectedSound).sound << std::endl;
    setActiveMixedSound(soundBindings.at(selectedSound));
  }

  if (includePanel){
    ImGui::End();
  }    
}

std::vector<std::string> listSoundFiles();
void renderMixingDetailPanel(bool includePanel){
  if (includePanel){
    ImGui::Begin("Mixing Detail");
  }

  auto mixedSoundName = activeMixedSound();
  if (!mixedSoundName.has_value()){
    if (includePanel){
      ImGui::End();
    }
    return;    
  }


  auto mixedSoundPtr = getMixedSound(mixedSoundName.value());
  auto& mixedSound = *mixedSoundPtr.value();


  ImGui::Text("Sound Name:");
  ImGui::SameLine();
  ImGui::Text(mixedSound.soundBinding.sound.c_str());

  if (ImGui::Button("Play")){
    playMixedSound(mixedSound.nameSymbol, std::nullopt);
  }
  ImGui::SameLine();
  if (ImGui::Button("Stop")){

  }
  ImGui::SameLine();
  if (ImGui::Button("Save")){
    saveMixedSound(mixedSound);
  }

  ImGui::Checkbox("Loop", &mixedSound.loop);
  ImGui::Checkbox("Center", &mixedSound.center);
  ImGui::SliderFloat("Volume", &mixedSound.volume, 0.f, 1.f);

  std::vector<std::string> clips = listSoundFiles();

  for (int i = 0; i < 5; i++){
    std::optional<std::string> currClip = mixedSound.clips.size() > i ? mixedSound.clips.at(i) : std::optional<std::string>(std::nullopt);
  
    bool enableSound = currClip.has_value();
    bool wasEnableSound = enableSound;

    std::string value = (std::string("Enable Sound ") + std::to_string(i));
    ImGui::Checkbox(value.c_str(), &enableSound);
    if (enableSound && !currClip.has_value()){
      currClip = clips.at(0);
      enableMixedSoundClip(mixedSound, i);
      std::cout << "enable curr clip" << std::endl;
    }
    if (!enableSound && wasEnableSound){
      disableMixedSoundClip(mixedSound, i);
      currClip = std::nullopt;
    }

    std::cout << "enable curr name: " << print(currClip) << std::endl;
    if (currClip.has_value()){
      std::cout << "enable curr drawCombo" << std::endl;

      if (ImGui::BeginCombo((std::string("Clip: ") + std::to_string(i)).c_str(), currClip.value().c_str())){
        for (int j = 0; j < clips.size(); j++){
          bool selected =  currClip.value() == clips.at(j);
          if (ImGui::Selectable(clips.at(j).c_str(), selected)){
            if (i == 0){
              setMixedSoundClip(mixedSound, clips.at(j), 0);
            }else if (i == 1){
              setMixedSoundClip(mixedSound, clips.at(j), 1);
            }else if (i == 2){
              setMixedSoundClip(mixedSound, clips.at(j), 2);
            }else if (i == 3){
              setMixedSoundClip(mixedSound, clips.at(j), 3);
            }else if (i == 4){
              setMixedSoundClip(mixedSound, clips.at(j), 4);
            }
          }
          if (selected){
            ImGui::SetItemDefaultFocus();
          }
        }
        ImGui::EndCombo();
      }
    }    
  }


  bool isSequential = mixedSound.clipOrderSequential;
  bool wasSequential = isSequential;
  bool isRandom = !mixedSound.clipOrderSequential;
  bool wasRandom = isRandom;

  ImGui::Checkbox("Sequential", &isSequential);
  ImGui::SameLine();
  ImGui::Checkbox("Random", &isRandom);
  if (isSequential && !wasSequential){
    mixedSound.clipOrderSequential = true;
  }else if (isRandom && !wasRandom){
    mixedSound.clipOrderSequential = false;
  }


  std::vector<std::string> buses = busNames();

  if (ImGui::BeginCombo("Bus", soundBusToStr(mixedSound.bus).c_str())){
    for (int i = 0; i < buses.size(); i++){
      bool selected = soundBusToStr(mixedSound.bus) == buses.at(i);
      if (ImGui::Selectable(buses.at(i).c_str(), selected)){
        auto selectedBusStr = buses.at(i);
        auto soundBus = stringToSoundBus(selectedBusStr);
        mixedSound.bus = soundBus;
      }
      if (selected){
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }

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

    registerAction("Start", "Mode", []() -> void {
      startMode(false);
    });
    registerAction("Stop", "Mode", []() -> void {
      stopMode(false);
      resetLevel();
    });


    registerView("Mixing", { "mixing" }, { "mixing-detail" });

    registerView("FPS", { "FPS - Weapons" }, { "FPS - Traits" });

}


