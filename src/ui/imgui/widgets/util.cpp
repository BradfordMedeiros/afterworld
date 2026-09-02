#include "./util.h"

extern CustomApiBindings* gameapi;
extern ConsoleInterface consoleInterface;

static bool showConsoleLog = false;

void renderConsole(bool includePanel){
  if (includePanel){
    ImGui::Begin("Console");
  }

  initializeConsole();

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
      IM_COL32(128 * opacity, 128 * opacity, 128 * opacity, 255 * 0.9f)
    );
  }

  auto& source = showConsoleLog ? logHistory : commandHistory; 
  if (ImGui::Button("Clear")){
      source.clear();
  }

  ImGui::Separator();

  // Scrollable output area
  ImGui::BeginChild("ConsoleOutput", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), true);

  static std::string consoleText;
  static std::vector<char> consoleBuffer;

  consoleText.clear();

  for (const auto& command : source){
      consoleText += command.command;
      consoleText += '\n';
  }

  consoleBuffer.assign(consoleText.begin(), consoleText.end());
  consoleBuffer.push_back('\0');

  ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));

  ImGui::InputTextMultiline(
    "##ConsoleOutputText",
    consoleBuffer.data(),
    consoleBuffer.size(),
    ImVec2(-FLT_MIN, -FLT_MIN),
    ImGuiInputTextFlags_ReadOnly
  );

  ImGui::PopStyleColor();

  ImGui::EndChild();
  ImGui::Separator();

  static char inputBuffer[256] = "";

  ImGui::SetNextItemWidth(-FLT_MIN);

  if (ImGui::InputText(
        "##ConsoleInput",
        inputBuffer,
        sizeof(inputBuffer),
        ImGuiInputTextFlags_EnterReturnsTrue))
  {
      std::cout << "value is: " << inputBuffer << std::endl;

      if (std::string(inputBuffer) == "log"){
        showConsoleLog = true;
      }else if (std::string(inputBuffer) == "console"){
        showConsoleLog = false;
      }

      executeCommand(consoleInterface, inputBuffer);

      inputBuffer[0] = '\0';
  }

  std::cout << "commandsize = " << commandHistory.size() << std::endl;
  if (includePanel){
    ImGui::End();
  }
}