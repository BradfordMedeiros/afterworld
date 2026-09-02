#include "./util.h"

extern CustomApiBindings* gameapi;
extern ConsoleInterface consoleInterface;

static bool showConsoleLog = false;


void renderConsole(bool includePanel){
  if (includePanel){
    ImGui::Begin("Console");
  }

  initializeConsole();

  static std::string textureName = "./res/textures/testgradient.png";
  static std::optional<GLuint> textureId;

  if (!textureId.has_value()){
    textureId = gameapi->getTextureSamplerId(textureName).value();
  }

  float opacity = 0.5f;

  ImVec2 windowPos = ImGui::GetWindowPos();
  ImVec2 windowSize = ImGui::GetWindowSize();

  ImGui::GetWindowDrawList()->AddImage(
    (ImTextureID)(intptr_t)textureId.value(),
    windowPos,
    ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y),
    ImVec2(0, 1),
    ImVec2(1, 0),
    IM_COL32(128 * opacity, 128 * opacity, 128 * opacity, 255 * 0.9f)
  );

  auto& source = showConsoleLog ? logHistory : commandHistory;

  if (ImGui::Button("Clear")){
    source.clear();
  }

  ImGui::Separator();

  ImGui::BeginChild("ConsoleOutput", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), true);

  static int selectedCommand = -1;

  for (int i = 0; i < source.size(); ++i){
    const auto& command = source.at(i);

    if (!command.valid){
      ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 0.f, 0.f, 1.f));
    }

    std::string label = command.command + "##" + std::to_string(i);

    if (ImGui::Selectable(label.c_str(), selectedCommand == i)){
      selectedCommand = i;
    }

    if (!command.valid){
      ImGui::PopStyleColor();
    }
  }

  if (selectedCommand >= 0 && selectedCommand < source.size() && ImGui::IsKeyPressed(ImGuiKey_C) && ImGui::GetIO().KeyCtrl){
    ImGui::SetClipboardText(source.at(selectedCommand).command.c_str());
  }

  ImGui::EndChild();

  ImGui::Separator();

  static char inputBuffer[256] = "";

  ImGui::SetNextItemWidth(-FLT_MIN);

  if (ImGui::InputText("##ConsoleInput", inputBuffer, sizeof(inputBuffer), ImGuiInputTextFlags_EnterReturnsTrue)){
    std::cout << "value is: " << inputBuffer << '\n';

    if (strcmp(inputBuffer, "log") == 0){
      showConsoleLog = true;
    }
    else if (strcmp(inputBuffer, "console") == 0){
      showConsoleLog = false;
    }

    executeCommand(consoleInterface, inputBuffer);

    inputBuffer[0] = '\0';
  }

  if (includePanel){
    ImGui::End();
  }
}