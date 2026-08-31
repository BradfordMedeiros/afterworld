#include "./settings.h"

extern CustomApiBindings* gameapi;


void resumeOnMenu();
void goToMenu();
std::vector<UILevel> queryLevels();
void goToLevel(std::string levelShortName);

void renderGraphicsPanel(bool includePanel){
  if (includePanel){
    ImGui::Begin("Fps Graphics");
  }

  float volume = 0.f;
  bool muteSound = false;
  if (ImGui::Checkbox("Enable Bloom", &muteSound)){

  }

  std::vector<std::string> resolutions {
    "1920x1080",
    "720x1080",
    "2560x1080",
  };
  if (ImGui::BeginCombo("Resolution", resolutions.at(0).c_str())){
      for (int i = 0; i < resolutions.size(); i++){
          bool selected = false;
          if (Mod::Selectable(resolutions.at(i).c_str(), selected)){
          }
          if (selected){
            ImGui::SetItemDefaultFocus();
          }
      }
      ImGui::EndCombo();
  }

  if(ImGui::SliderFloat("Field of View", &volume, 45.0f, 115.0f)){
  }

  if (includePanel){
    ImGui::End();
  }    
}


int toGlfwKey(ImGuiKey key) {
    if (key >= ImGuiKey_A && key <= ImGuiKey_Z) {
        return GLFW_KEY_A + (key - ImGuiKey_A);
    }

    if (key >= ImGuiKey_0 && key <= ImGuiKey_9) {
        return GLFW_KEY_0 + (key - ImGuiKey_0);
    }

    switch (key) {
        case ImGuiKey_Space:        return GLFW_KEY_SPACE;

        case ImGuiKey_LeftShift:    return GLFW_KEY_LEFT_SHIFT;
        case ImGuiKey_RightShift:   return GLFW_KEY_RIGHT_SHIFT;

        case ImGuiKey_LeftCtrl:     return GLFW_KEY_LEFT_CONTROL;
        case ImGuiKey_RightCtrl:    return GLFW_KEY_RIGHT_CONTROL;

        case ImGuiKey_LeftAlt:      return GLFW_KEY_LEFT_ALT;
        case ImGuiKey_RightAlt:     return GLFW_KEY_RIGHT_ALT;

        case ImGuiKey_CapsLock:     return GLFW_KEY_CAPS_LOCK;

        case ImGuiKey_Enter:        return GLFW_KEY_ENTER;
        case ImGuiKey_Tab:          return GLFW_KEY_TAB;
        case ImGuiKey_Backspace:    return GLFW_KEY_BACKSPACE;
        case ImGuiKey_Escape:       return GLFW_KEY_ESCAPE;

        default:
            return 0;
    }
}


bool renderKeyBinding(const char* title, int* keyBinding, bool isWaiting){
  std::string buttonLabel(title);
  buttonLabel += " - " + std::to_string(*keyBinding);

  if (Mod::Button(buttonLabel.c_str(), ImVec2(120, 0))) {
    return true;
  }
  if (isWaiting){
    ImGui::SameLine();
    ImGui::Text("Press a key...");
  }
  return false;
}


void renderControlsPanel(bool includePanel){
  if (includePanel){
    ImGui::Begin("Fps Controls");
  }

    float volume = 0.f;
    bool muteSound = false;
    if (ImGui::Checkbox("Controls", &muteSound)){

    }

    static std::optional<std::string> currentKey;

    auto controls = controlBindings();
    for (auto& control : controls){
      bool isWaiting = currentKey.has_value() && currentKey.value() == control.text;
      auto clickedBinding = renderKeyBinding(control.text.c_str(), control.currentKey, isWaiting);
      if (clickedBinding){
        currentKey = control.text;
      }
    }

    if (currentKey.has_value()) {
      for (int key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; key++) {
          if (ImGui::IsKeyPressed((ImGuiKey)key)) {
              auto ascii = toGlfwKey((ImGuiKey)key);
              for (auto& control : controls){
                if (control.text == currentKey.value()){
                  *control.currentKey = ascii;
                  playMixedSound(getSymbol("interaction/activate"), std::nullopt);
                }
              }
              currentKey = std::nullopt;
              break;
          }
      }
    }

    if(ImGui::SliderFloat("Mouse Sensitivity ", &volume, 0.0f, 10.0f)){
    }

    if (Mod::Button("Restore Defaults")) {
        ImGui::OpenPopup("Confirm Restore");
    }
    if (ImGui::BeginPopupModal("Confirm Restore", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar)) {
        ImGui::Text("Are you sure you want to restore all defaults?");
        if (Mod::Button("Yes")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (Mod::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (includePanel){
      ImGui::End();
    }    
}

void renderGameVolumePanel(bool includePanel){
  if (includePanel){
    ImGui::Begin("Fps Volume");
  }

    float volume = 0.f;
    bool muteSound = isMuted();


    if (ImGui::Checkbox("Mute Sound", &muteSound)){
      setIsMuted(muteSound);
    }

    if(ImGui::SliderFloat("Master Volume ", &volume, 0.0f, 1.0f)){
    }

    auto gameplayVolume = getGameplayVolume();
    if(ImGui::SliderFloat("Gameplay Volume ", gameplayVolume, 0.0f, 1.0f)){
    }

    auto musicVolume = getMusicVolume();
    if(ImGui::SliderFloat("Music Volume ", musicVolume, 0.0f, 1.0f)){
    }
  if (includePanel){
    ImGui::End();
  }    
}

std::string selectedSettingOption = "Graphics";

void renderGameSettingsControlPanel(bool includePanel){
  if (includePanel){
    ImGui::Begin("Settings");
  }
    float volume = 0.f;
    bool muteSound = false;

    std::vector<std::string> items {
        "Graphics",
        "Controls",
        "Sound",
    };
    for (auto& item : items) {
    if (Mod::Selectable(item.c_str(), false, 0, ImVec2(300, 40))) {
      selectedSettingOption = item;
    }
  }
  if (includePanel){
    ImGui::End();
  }    
}

void renderGameSettingsView(bool includePanel){
  if (selectedSettingOption == "Graphics"){
    renderGraphicsPanel(includePanel);
  }else if (selectedSettingOption == "Controls"){
    renderControlsPanel(includePanel);
  }else if (selectedSettingOption == "Sound"){
    renderGameVolumePanel(includePanel);
  }
}

struct MenuItem {
  std::string text;
  std::function<void()> onClick;
};


void renderList(bool includePanel, const char* title, std::vector<MenuItem>& menuItems, bool centerText, ImFont* font = NULL){
    if (includePanel){
      ImGui::Begin(title, nullptr, ImGuiWindowFlags_NoBackground);
    }

    ImGuiIO& io = ImGui::GetIO();
    static ImFont* defaultFont = io.Fonts->AddFontFromFileTTF("./res/fonts/vcr.ttf", 32.f);
    static int selected = 0;

    ImFont* bigFont = font ? font : defaultFont;
 
    ImGui::PushFont(bigFont);
    ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(1, 1, 1, 0.6));
    ImGui::PushStyleColor(ImGuiCol_Header,        ImVec4(0.15f, 0.15f, 0.15f, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.0f, 0.f, 1.0f, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive,  ImVec4(1.f, 0.f, 0.f, 1.0f));

    bool fullWidth = true;
    float menuWidth =  fullWidth ? ImGui::GetContentRegionAvail().x : 0.f;
    for (int i = 0; i < menuItems.size(); ++i){
      menuWidth = std::max(menuWidth, ImGui::CalcTextSize(menuItems.at(i).text.c_str()).x);
    }
        
    float padding = 0.0f;
    float itemWidth = menuWidth + padding * 2.0f;
    float panelWidth = ImGui::GetContentRegionAvail().x;

    // This centers the actual text
    ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));

    for (int i = 0; i < menuItems.size(); ++i){
        if (centerText){
          ImGui::SetCursorPosX(ImGui::GetStyle().WindowPadding.x + (panelWidth - itemWidth) * 0.5f);
        }
        if (ImGui::Selectable(menuItems.at(i).text.c_str(), selected == i, 0, ImVec2(itemWidth, 0.f))){
          selected = i;
          menuItems.at(i).onClick();
        }
        if (ImGui::IsItemHovered()){
          selected = i;
        }
            
    }

    ImGui::PopStyleVar();
    ImGui::PopFont();
    ImGui::PopStyleColor(4);

    if (includePanel){
        ImGui::End();
    }
}


// void pushHistory(std::vector<std::string> route, bool replace, std::optional<std::any> data, bool forceReload);

void renderMainMenu(bool includePanel){
    ImGuiIO& io = ImGui::GetIO();
    static ImFont* bigFont = io.Fonts->AddFontFromFileTTF("./res/fonts/panoptic.otf", 30.0f);

    ImGui::PushFont(bigFont);

    ImVec2 textSize = ImGui::CalcTextSize("Afterworld");
    float width = ImGui::GetContentRegionAvail().x;
    ImGui::SetCursorPosX(
        ImGui::GetStyle().WindowPadding.x +
        (width - textSize.x) * 0.5f
    );

    ImGui::Text("Afterworld");
    ImGui::PopFont();
    ImGui::Dummy(ImVec2(0, 20));

    static std::vector<MenuItem> menuItems {
      MenuItem { 
        .text = "Campaign",
        .onClick = []() -> void {
          pushHistory({ "levelselect" }, false, std::nullopt, false);
        },
      },
      MenuItem { 
        .text = "Settings",
        .onClick = []() -> void {
          pushHistory({ "settings" }, false, std::nullopt, false);
        },
      },
      MenuItem { 
        .text = "Exit",
        .onClick = []() -> void {
          exit(0);
        },
      },
    };
    renderList(includePanel, "Main Panel", menuItems, true);
}

void renderMainMenu2(bool includePanel, LiveMenuFn& liveMenu){
    ImGuiIO& io = ImGui::GetIO();
    static ImFont* bigFont = io.Fonts->AddFontFromFileTTF("./res/fonts/panoptic.otf", 30.0f);

    ImGui::PushFont(bigFont);

    ImVec2 textSize = ImGui::CalcTextSize("The Pyramid");
    float width = ImGui::GetContentRegionAvail().x;
    ImGui::SetCursorPosX(
        ImGui::GetStyle().WindowPadding.x +
        (width - textSize.x) * 0.5f
    );

    ImGui::Text("The Pyramid");
    ImGui::PopFont();
    ImGui::Dummy(ImVec2(0, 20));

    static std::vector<MenuItem> menuItems {
      MenuItem { 
        .text = "New Game",
        .onClick = liveMenu.newGame,
      },
      MenuItem { 
        .text = "Continue",
        .onClick = liveMenu.continueGame,
      },
      MenuItem { 
        .text = "Quit",
        .onClick = []() -> void {
          exit(0);
        },
      },
    };
    renderList(includePanel, "Main Panel", menuItems, true);
}


void renderPauseMenu(bool includePanel){
    static std::vector<MenuItem> menuItems {
      MenuItem { 
        .text = "Resume",
        .onClick = resumeOnMenu,
      },
      MenuItem { 
        .text = "Go to Menu",
        .onClick = goToMenu,
      },
    };
    renderList(includePanel, "Pause Panel", menuItems, true);
}


void renderDeadMenu(bool includePanel){
    static std::vector<MenuItem> menuItems {
      MenuItem { 
        .text = "Go to Menu",
        .onClick = goToMenu,
      },
    };
    renderList(includePanel, "Game Over", menuItems, true);
}


static std::optional<UILevel> selectedLevel;

void renderLevelList(bool includePanel){
    if (includePanel){
      ImGui::Begin("Level List", nullptr, ImGuiWindowFlags_NoBackground);
    }

    ImVec2 available = ImGui::GetContentRegionAvail();
    ImVec2 cursor = ImGui::GetCursorPos();
    ImVec2 childSize(available.x * 0.75f,  available.y * 0.75f);

    ImGui::SetCursorPos(ImVec2(cursor.x + (available.x - childSize.x) * 0.5f, cursor.y + (available.y - childSize.y) * 0.5f));

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f, 0.0f, 0.0f, 0.5f));

    ImGui::BeginChild("level-list-child", childSize);

 //   ImVec2 cursor = ImGui::GetCursorPos();
 //   ImGui::SetCursorPos(ImVec2(cursor.x + (ImGui::GetContentRegionAvail().x - panelWidth * 0.5f), cursor.y));

    ImGuiIO& io = ImGui::GetIO();
    ImFontConfig config;
    config.SizePixels = 12.0f;
    static ImFont* smallFont = io.Fonts->AddFontDefault(&config);

    ImGui::PushFont(smallFont);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 1.f, 1.f));
    ImGui::Text("Levels");
    ImGui::PopStyleColor();
    ImGui::PopFont();

    ImGui::Dummy(ImVec2(0, 20));


    auto levels = queryLevels();
    std::vector<MenuItem> menuItems;
    for (auto& level : levels){
      menuItems.push_back(MenuItem {
        .text = level.name,
        .onClick = [level]() -> void {
          selectedLevel = level;
        },
      });
    }

    renderList(false, "Level List", menuItems, true, smallFont);


    ImVec2 size = ImVec2(ImGui::GetContentRegionAvail().x, 50);
    float availableWidth = ImGui::GetContentRegionAvail().x;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (availableWidth - size.x) * 0.5f);

    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.5f, 0.5f));

    ImGui::BeginDisabled(!selectedLevel.has_value());
    if (Mod::Button("Start", size)) {
      if (selectedLevel.has_value()){
        goToLevel(selectedLevel.value().shortcut);
      }
    }
    ImGui::EndDisabled();
  
    ImGui::PopStyleVar();

    ImGui::EndChild();

    ImGui::PopStyleColor();

    if (includePanel){
        ImGui::End();
    }

}


void renderLevelDetail(bool includePanel){
    if (includePanel){
        ImGui::Begin("Level Detail", nullptr, ImGuiWindowFlags_NoBackground);
    }

    ImVec2 available = ImGui::GetContentRegionAvail();
    ImVec2 cursor = ImGui::GetCursorPos();

    float imageWidth = available.x * 0.75f;
    float imageHeight = imageWidth * 9.0f / 16.0f;
    ImVec2 imageSize(imageWidth, imageHeight);

    std::string defaultTextureName = "./res/textures/wood.jpg";

    auto textureId = gameapi->getTextureSamplerId(
        selectedLevel.has_value()
            ? selectedLevel.value().image
            : defaultTextureName
    ).value();

    // Center image horizontally and vertically
    ImGui::SetCursorPos(ImVec2(
        cursor.x,
        cursor.y + (available.y - imageHeight) * 0.333f
    ));

    ImGui::Image(
        (ImTextureID)(intptr_t)textureId,
        imageSize,
        ImVec2(0, 1),
        ImVec2(1, 0)
    );

    //
    // DETAILS
    //

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (selectedLevel.has_value()) {
        auto& level = selectedLevel.value();

        ImGui::Text("%s", level.name.c_str());

        ImGui::Spacing();

        ImGui::TextWrapped(
            "%s",
            level.description.c_str()
        );

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("Highest Score: %d", 1000);

        ImGui::Text("Difficulty: %s", "Hard");

        ImGui::Text(
            "Image: %s",
            selectedLevel.value().image.c_str()
        );

    } else {
        ImGui::TextDisabled("Select a level");
    }


    if (includePanel){
        ImGui::End();
    }
}