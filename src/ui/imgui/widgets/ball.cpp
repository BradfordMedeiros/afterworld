#include "./ball.h"

void reloadVehicleSettings();

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

void renderBallProgressInfo(bool includePanel, BallModeUi& ballModeUi){
  if (includePanel){
    ImGui::Begin("renderBallProgressInfo");
  }

  if (ballModeUi.levelSelect.has_value()){
    ImFont* font = getFontByBinding(getSymbol("ball-level-info"));
    ImGui::PushFont(font);

    std::string level = std::string("Level: ") + ballModeUi.levelSelect.value().level;

    float panelWidth = ImGui::GetContentRegionAvail().x;
    float width = ImGui::CalcTextSize(level.c_str()).x;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (panelWidth - width) * 0.5f);
    ImGui::TextUnformatted(level.c_str());

    const char* text = "par time: ";
    std::string parTime = std::string(text) + ballModeUi.levelSelect.value().parTime;
    width = ImGui::CalcTextSize(parTime.c_str()).x;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (panelWidth - width) * 0.5f);
    ImGui::TextUnformatted(parTime.c_str());

    std::string bestTime = std::string("best time: ") + ballModeUi.levelSelect.value().bestTime;
    width = ImGui::CalcTextSize(bestTime.c_str()).x;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (panelWidth - width) * 0.5f);
    ImGui::TextUnformatted(bestTime.c_str());

    std::string gems = "total gems " + std::to_string(ballModeUi.levelSelect.value().gems) + " / " + std::to_string(ballModeUi.levelSelect.value().totalGems);
    width = ImGui::CalcTextSize(gems.c_str()).x;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (panelWidth - width) * 0.5f);
    ImGui::TextUnformatted(gems.c_str());

    ImGui::PopFont();
  }

  ImFont* font = getFontByBinding(getSymbol("ball-level-complete"));
  ImGui::PushFont(font);
  
  if (ballModeUi.ballMode.levelComplete.has_value()){
      const char* text = "Level Complete";
      float width = ImGui::CalcTextSize(text).x;
      ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - width) * 0.5f);
      ImGui::TextUnformatted(text);
  
      text = "Click to Continue";
      width = ImGui::CalcTextSize(text).x;
      ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - width) * 0.5f);
      ImGui::TextUnformatted(text);
  }else if (ballModeUi.ballMode.showComplete){
      const char* text = "Level Complete, Moving";
      float width = ImGui::CalcTextSize(text).x;
      ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - width) * 0.5f);
      ImGui::TextUnformatted(text);
  }
  
  ImGui::PopFont();

  if (ballModeUi.ballMode.showElapsedTime && ballModeUi.ballMode.elapsedTime.has_value()){
    std::string text = std::to_string(ballModeUi.ballMode.elapsedTime.value()());
    float width = ImGui::CalcTextSize(text.c_str()).x;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - width) * 0.5f);
    ImGui::Text(text.c_str());
  }

  /*
      if (ballOptions -> showPowerup){
      if (ballOptions -> powerupTexture.has_value()){
        auto powerupUsed = ballOptions -> powerupStartTime.has_value();
        drawTools.drawRect(0.8f, 0.8f, 0.2f, 0.2f, false, glm::vec4(1.f, 1.f, 1.f, powerupUsed ? 0.2 : 0.9f), true, std::nullopt, ballOptions -> powerupTexture.value(), std::nullopt, std::nullopt);
      }
      if (ballOptions -> powerupDuration.has_value()){
        auto elapsedTime = gameapi -> timeSeconds(false) - ballOptions -> powerupStartTime.value();
        auto percentage = 1.f - (elapsedTime / ballOptions -> powerupDuration.value());
        if (percentage < 0){
          percentage = 0.f;
        }
        drawTools.drawRect(0.8f, 0.7f, 0.1f * percentage, 0.02f, false, glm::vec4(1.f, 1.f, 1.f, 0.9f), true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
      }
    }

  */

  //  if (ballOptions -> showElapsedTime && ballOptions -> elapsedTime.has_value()){

  if (includePanel){
    ImGui::End();
  }
}


void renderStageSelectPanel(bool includePanel){
    if (includePanel){
        ImGui::Begin("renderStageSelectPanel");
    }

    ImGui::Text("Stage Select");

    const int columns = 4;
    const float size = 80.0f;
    const float spacing = 10.0f;

    for (int i = 0; i < 12; i++){
        int row = i / columns;
        int column = i % columns;

        ImGui::SetCursorPos(ImVec2(
            column * (size + spacing),
            40.0f + row * (size + spacing)
        ));

        ImGui::Button(
            std::to_string(i + 1).c_str(),
            ImVec2(size, size)
        );
    }

    if (includePanel){
        ImGui::End();
    }
}