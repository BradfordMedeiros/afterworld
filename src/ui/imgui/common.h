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
    ImGui::Button(title, vec);
    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)){
      playMixedSound(getSymbol("screens/menuclick"), std::nullopt);
      return true;
    }
    return false;
  }

  inline bool Selectable(const char* label, bool selected = false,  ImGuiSelectableFlags flags = 0, const ImVec2& size = ImVec2(0, 0)){
    ImGui::Selectable(label, selected, flags, size);
    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)){
      playMixedSound(getSymbol("screens/menuclick"), std::nullopt);
      return true;
    }
    return false;

  }
}


