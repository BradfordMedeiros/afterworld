#include "./util.h"

extern CustomApiBindings* gameapi;

std::vector<std::string> consoleLines {
  "line 1",
  "this is line 2",
  "line3",
};
void renderConsole(bool includePanel){
  if (includePanel){
    ImGui::Begin("Console");
  }

  float opacity = 0.5f;
  {
    std::string textureName = "./res/textures/testgradient.png";
    auto textureId = gameapi->getTextureSamplerId(textureName).value();

    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();

    ImGui::GetWindowDrawList()->AddImage(
      (ImTextureID)(intptr_t)textureId,
      windowPos,
      ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y),
      ImVec2(0, 1),
      ImVec2(1, 0),
      IM_COL32(128  * opacity, 128 * opacity, 128 * opacity, 255 * 0.9f)
    );
  }

  if (ImGui::Button("Clear")){
      consoleLines.clear();
  }

  ImGui::Separator();

  // Scrollable output area
  ImGui::BeginChild("ConsoleOutput", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), true);

  for (const auto& line : consoleLines){
      ImGui::TextUnformatted(line.c_str());
  }

  ImGui::EndChild();
  ImGui::Separator();

  // Input
  static char inputBuffer[256] = "";

  ImGui::SetNextItemWidth(-FLT_MIN);
  if (ImGui::InputText("##ConsoleInput", inputBuffer, sizeof(inputBuffer), ImGuiInputTextFlags_EnterReturnsTrue)){
      // Process command here
      //ProcessCommand(inputBuffer);
      inputBuffer[0] = '\0';
  }





  if (includePanel){
    ImGui::End();
  }   
}