#include "./editorui.h"

extern CustomApiBindings* gameapi;

void startMode(bool loadedScene);
void stopMode(bool unloadedScene);

void resetLevel();
void updateArcadeObj(objid id, std::string newType);
void rebootMachine(objid id);

std::optional<std::string> ScenegraphView(std::string directory, FILE_EXTENSION_TYPE type);

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

  auto selectedFile = ScenegraphView("../gameresources/textures/", IMAGE_EXTENSION);
  if (selectedFile.has_value()){
    std::cout << "selected file: " << selectedFile.value() << std::endl;
    setGameObjectTexture(objectToDetail.value(), selectedFile.value());
  }

  if (includePanel){
    ImGui::End();
  }     
}



UiSettings uiSettings{};
UiSettings* getUiSettings(){
  return &uiSettings;
}

// pos 0.5 is center, 1.f is right, 0.f is left
// pos 0.5 is center, 1.f is up, 0.f is down
// alignment -> 0,5 is center, 1.f is right, 0.f is left
//            ->0.5 is center, 1.f is up, 0.f is down

void renderBackground(const char* name, glm::vec4 tint = glm::vec4(1.f, 1.f, 1.f, 1.f), float widthPercent = 1.f, float heightPercent = 1.f, std::optional<std::string> texture = std::optional<std::string>(std::nullopt)){
    return;
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x * widthPercent, viewport->Size.y * heightPercent));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove ;
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.f, 0.f, 0.f, 0.f));

    ImGui::Begin(name, nullptr, flags);

    {
      std::string defaultTextureName = texture.has_value() ? texture.value() : "./res/textures/testgradient.png";
      auto textureId = gameapi->getTextureSamplerId(defaultTextureName).value();

      // Center image horizontally and vertically
      //ImGui::SetCursorPos(ImVec2(cursor.x, cursor.y + (available.y - imageHeight) * 0.333));

      ImGui::GetWindowDrawList()->AddImage(
          (ImTextureID)(intptr_t)textureId,
          viewport->Pos,
          ImVec2(
              viewport->Pos.x + viewport->Size.x,
              viewport->Pos.y + viewport->Size.y
          ),
          ImVec2(0, 1),
          ImVec2(1, 0),
          IM_COL32(255 * tint.x, 255 * tint.y, 255 * tint.z, 255 * tint.w)
      );
    }

    ImGui::End();

    ImGui::PopStyleColor();
}


void renderMoreUi(){
  if (false && uiSettings.showFpsHud){
    auto& widget = *widgetByNameSymbol(getSymbol("FPS - Hud")).value();
    renderLayoutCenter("fps-hud-layout", widget);
  }

  if (uiSettings.showGameSettings){
    auto view = viewByName(getSymbol("GameSettings"));
    // drawTools.drawRect(0.f, 0.f, 2.f, 2.f, false, glm::vec4(0.2f, 0.2f, 0.2f, opacity), true, std::nullopt, "../gameresources/build/textures/evilpattern.png", std::nullopt, std::nullopt);
    renderBackground("game-settings-background", glm::vec4(0.3f, 0.3f, 0.3f, 0.6f), 1.f, 1.f,  "../gameresources/build/textures/evilpattern.png");

    renderLayout(*view.value());    
  }
  
  
  if (uiSettings.showMainMenu){
      auto& widget = *widgetByNameSymbol(getSymbol("main-menu")).value();
      renderLayoutAlignUpCenterHorz("main-menu-layout", widget, ImVec2(0.5f, 0.5f), ImVec2(0.5f, 0.5f), ImVec2(700.f, 500.f));
  }

  if (uiSettings.showPauseMenu){
    renderBackground("##pausemenu-background");
    auto& widget = *widgetByNameSymbol(getSymbol("pause-menu")).value();
    renderLayoutAlignUpCenterHorz("pause-menu-layout", widget, ImVec2(0.5f, 0.5f), ImVec2(0.5f, 0.5f), ImVec2(300.f, 100.f));
  }

  if (uiSettings.showDeadMenu){
    renderBackground("##deadmenu-background");
    auto& widget = *widgetByNameSymbol(getSymbol("dead-menu")).value();
    renderLayoutAlignUpCenterHorz("dead-menu-layout", widget, ImVec2(0.5f, 0.5f), ImVec2(0.5f, 0.5f), ImVec2(300.f, 100.f));
  }

  if (uiSettings.showLevelSelect){
    renderBackground("level-select-background", glm::vec4(0.3f, 0.3f, 0.3f, 0.6f), 1.f, 1.f,  "../gameresources/build/textures/evilpattern.png");

    ImVec2 screen = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(screen);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

    ImGui::Begin("level-select-layout", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove  | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus);

    auto& widgetList = *widgetByNameSymbol(getSymbol("level-list")).value();
    auto& widgetDetail = *widgetByNameSymbol(getSymbol("level-detail")).value();

    renderLayoutHalf(widgetList, widgetDetail);

    ImGui::End();

    ImGui::PopStyleVar(2);
  }

  if (uiSettings.liveMenu.has_value()){
      {
        auto& widget = *widgetByNameSymbol(getSymbol("main-menu2")).value();
        renderLayoutAlignUpCenterHorz("main-menu2-livemenu-layout", widget, ImVec2(0.5f, 0.5f), ImVec2(0.5f, 0.5f), ImVec2(700.f, 500.f));
      }
  }

  if (uiSettings.ballModeUi){
      {
        if (uiSettings.ballModeUi -> ballMode.levelComplete.has_value()){
          renderBackground("##levelcomplete-background");
        }
        auto& widget = *widgetByNameSymbol(getSymbol("game-ball-progress")).value();
        renderLayoutAlignUpCenterHorz("main-menu2-ball-progress-layout", widget, ImVec2(0.f, 0.5f), ImVec2(1.f, 0.5f), ImVec2(700.f, 500.f));
      }
  }

  if (uiSettings.showTerminal){
    auto& widget = *widgetByNameSymbol(getSymbol("terminal")).value();
    renderLayoutCenter("terminal-layout", widget);
  }

  if (true){
    auto& widget = *widgetByNameSymbol(getSymbol("navigation")).value();

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove;
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.f, 0.f, 1.f, 0.f));
    ImGui::Begin("navigation-layoutwindow", nullptr, flags);
      renderLayoutAlignUpCenterHorz("nav-layout", widget, ImVec2(0.f, 0.f), ImVec2(1.f, 0.5f), ImVec2(100.f, 200.f));
    ImGui::End();

    ImGui::PopStyleColor();
  }

  static std::optional<float> showConsoleTime;
  if (uiSettings.showConsole){
    if (!showConsoleTime.has_value()){
      showConsoleTime = gameapi -> timeSeconds(true);
    }
    float elapsedTime = gameapi -> timeSeconds(true) - showConsoleTime.value();

    auto& widget = *widgetByNameSymbol(getSymbol("console")).value();

    float sizeRatio = 0.75f;
    float percentage = glm::min(1.f, elapsedTime / 0.25f);
    //renderBackground("##console-background", percentage, 1.f, sizeRatio);
    {
      //renderLayoutCenter("console-layout", widget);
      auto size = ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y * sizeRatio);
      renderLayoutAlignUpCenterHorz("console-layout", widget, ImVec2(1.f, 1.f), ImVec2(0.f, 1.f - percentage), size);
    }
      
  }else{
    showConsoleTime = std::nullopt;
  }

  
}


void initImGuiGameUi(){
    // Systems
    { 
      registerWidget("level", "system", [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
          renderLevelPanel(includePanel);
      });   
      registerWidget("Trigger", "system", [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
          renderTriggerPanel(includePanel);
      });   
      registerWidget("animations", "system", [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
          renderAnimations(includePanel);
      });  
      registerWidget("debug-gametype", "system", [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
          renderGameType(includePanel);
      });  
      registerWidget("debug-hitpoints", "system", [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
          renderHitpoints(includePanel);
      });  
      registerWidget("debug-inventory", "system", [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
          renderInventory(includePanel);
      });  
    }

    // Fps
    { 
      std::optional<std::string> weaponsUi = std::nullopt;
      registerWidget("FPS - Weapons", weaponsUi, [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
          renderWeaponsPanel(includePanel);
      });       
      registerWidget("FPS - Traits", weaponsUi, [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
          renderTraitsPanel(includePanel);
      });
      registerWidget("Hud", weaponsUi, [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
          renderFpsHud(includePanel);
      });       
      registerWidget("Spawn", "fps", [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
          renderSpawnPanel(includePanel);
      });     
      registerWidget("Props", "fps", [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
          renderPropPanel(includePanel, sceneId);
      }); 
      registerWidget("arcade", "fps", [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
          renderArcade(includePanel, objectToDetail, sceneId);
      });
    }

    // Ball
    { 
      registerWidget("Game - Ball", "ball", [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
          renderBallGameplay(includePanel);
      });     
      registerWidget("game-ball-progress", std::nullopt, [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
          if (getUiSettings() -> ballModeUi){
            renderBallProgressInfo(includePanel, *getUiSettings() -> ballModeUi);
          }
      });     
    }

    // Audio and mixing
    { 
      registerWidget("mixing", std::nullopt, [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
          renderMixingPanel(includePanel);
      });
      registerWidget("mixing-detail", std::nullopt, [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
          renderMixingDetailPanel(includePanel);
      });  
    }

    // in game ui
    { 
      //std::optional<std::string> menu = "in-game-ui";
      std::optional<std::string> menu;
      registerWidget("main-menu", menu, [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
          renderMainMenu(includePanel);
      });
      registerWidget("main-menu2", menu, [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
          if (getUiSettings() -> liveMenu.has_value()){
            renderMainMenu2(includePanel, getUiSettings() -> liveMenu.value());
          }
      });
      registerWidget("pause-menu", menu, [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
          renderPauseMenu(includePanel);
      });
      registerWidget("dead-menu", menu, [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
          renderDeadMenu(includePanel);
      });
      registerWidget("terminal", menu, [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
          renderTerminal(includePanel);
      });
      registerWidget("navigation", menu, [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
          renderNavigation(includePanel);
      });  
      registerWidget("level-list", menu, [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
          renderLevelList(includePanel);
      });  
      registerWidget("level-detail", menu, [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
          renderLevelDetail(includePanel);
      });
      registerWidget("console", menu, [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
        renderConsole(includePanel);
      });  
      registerWidget("game-settings-select", menu, [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
          renderGameSettingsControlPanel(includePanel);
      });
      registerWidget("game-settings", menu, [](bool includePanel, std::optional<objid> objectToDetail, std::optional<objid> sceneId) -> void {
          renderGameSettingsView(includePanel);
      });
    }


    registerAction("Start", "Mode", []() -> void {
      startMode(false);
    });
    registerAction("Stop", "Mode", []() -> void {
      stopMode(false);
      resetLevel();
    });


    registerView("Mixing", false,  { "mixing" }, { "mixing-detail" }, DIVIDED_LAYOUT);
    registerView("FPS", false, { "FPS - Weapons" }, { "FPS - Traits" }, DIVIDED_LAYOUT);
    registerView("GameSettings", true,{ "game-settings-select" }, { "game-settings" }, SPLIT_LAYOUT);

    setGuiFn(renderMoreUi);
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