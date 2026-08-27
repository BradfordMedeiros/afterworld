#pragma once

#include "../../../../ModEngine/src/ui/gui.h"
#include "../../scene_routing.h"

namespace Mod {
  inline bool Button(const char* title){
    if(ImGui::Button(title)){
      playMixedSound(getSymbol("screens/menuclick"), std::nullopt);
      return true;
    }
    return false;
  }

  inline bool Button(const char* title, ImVec2 vec){
    if(ImGui::Button(title, vec)){
      playMixedSound(getSymbol("screens/menuclick"), std::nullopt);
      return true;
    }
    return false;
  }

  inline bool Selectable(const char* label, bool selected = false,  ImGuiSelectableFlags flags = 0, const ImVec2& size = ImVec2(0, 0)){
    if(ImGui::Selectable(label, selected, flags, size)){
      playMixedSound(getSymbol("screens/menuclick"), std::nullopt);
      return true;
    }
    return false;
  }
}

