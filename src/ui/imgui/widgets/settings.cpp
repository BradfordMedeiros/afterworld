#include "./settings.h"

void resumeOnMenu();
void goToMenu();

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
    bool muteSound = false;
    if (ImGui::Checkbox("Mute Sound", &muteSound)){

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


void renderList(bool includePanel, const char* title, std::vector<MenuItem>& menuItems){
    if (includePanel){
      ImGui::Begin(title, nullptr, ImGuiWindowFlags_NoBackground);
    }

    ImGuiIO& io = ImGui::GetIO();
    static ImFont* bigFont = io.Fonts->AddFontFromFileTTF("./res/fonts/Walby-Regular.ttf", 32.0f);

    static int selected = 0;

 
    ImGui::PushFont(bigFont);
    ImGui::PushStyleColor(ImGuiCol_Header,        ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.30f, 0.30f, 0.30f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive,  ImVec4(0.45f, 0.45f, 0.45f, 1.0f));

    float menuWidth = 0.0f;
    for (int i = 0; i < menuItems.size(); ++i){
      menuWidth = std::max(menuWidth, ImGui::CalcTextSize(menuItems.at(i).text.c_str()).x);
    }
        

    float padding = 30.0f;
    float itemWidth = menuWidth + padding * 2.0f;
    float panelWidth = ImGui::GetContentRegionAvail().x;

    // This centers the actual text
    ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));

    for (int i = 0; i < menuItems.size(); ++i){
        // This centers the panael
        ImGui::SetCursorPosX(ImGui::GetStyle().WindowPadding.x + (panelWidth - itemWidth) * 0.5f);
        //ImGui::SetCursorPosX(ImGui::GetStyle().WindowPadding.x + (panelWidth - itemWidth) * 0.5f);

        if (ImGui::Selectable(menuItems.at(i).text.c_str(), selected == i, 0, ImVec2(itemWidth, 0))){
          selected = i;
          menuItems.at(i).onClick();
        }
        if (ImGui::IsItemHovered()){
          selected = i;
        }
            
    }

    ImGui::PopStyleVar();
    ImGui::PopFont();
    ImGui::PopStyleColor(3);

    if (includePanel){
        ImGui::End();
    }
}



void renderMainMenu(bool includePanel){
    static std::vector<MenuItem> menuItems {
      MenuItem { 
        .text = "CAMPAIGN",
        .onClick = []() -> void {
          std::cout << "list: CAMPAIGN clicked" << std::endl;
        },
      },
      MenuItem { 
        .text = "SETTINGS",
        .onClick = []() -> void {
          std::cout << "list: SETTINGS clicked" << std::endl;
    
        },
      },
      MenuItem { 
        .text = "EXIT",
        .onClick = []() -> void {
          std::cout << "list: EXIT clicked" << std::endl;
        },
      },
    };
    renderList(includePanel, "Main Panel", menuItems);
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
    renderList(includePanel, "Pause Panel", menuItems);
}
