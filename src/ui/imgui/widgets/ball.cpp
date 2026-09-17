#include "./ball.h"

extern CustomApiBindings* gameapi;

void reloadVehicleSettings();
void goToLevel(std::string);

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



void addLevelDots(float cursorX, float availableWidth, int levelsPerWorld, int selectedLevel){
    ImGui::Spacing();

    const float progressionWidth = 240.0f;

    float progressionX = cursorX + (availableWidth - progressionWidth) * 0.5f;

    ImGui::SetCursorPosX(progressionX);

    ImVec2 progressionStart = ImGui::GetCursorScreenPos();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    float dotSpacing = progressionWidth / (levelsPerWorld - 1);

    for (int i = 0; i < levelsPerWorld; i++){
        float x = progressionStart.x + dotSpacing * i;
        float y = progressionStart.y + 5.0f;

        bool completed = i < 2;
        bool selected = i == selectedLevel;
        bool locked = i > 2;

        float radius = selected ? 5.0f : 3.0f;

        ImU32 color;

        if (selected){
            color = IM_COL32(255, 255, 255, 255);
        }
        else if (completed){
            color = IM_COL32(255, 255, 255, 180);
        }
        else if (locked){
            color = IM_COL32(255, 255, 255, 60);
        }
        else{
            color = IM_COL32(255, 255, 255, 120);
        }

        if (locked){
            drawList->AddCircle(ImVec2(x, y), radius, color, 16, 1.5f);
        }
        else{
            drawList->AddCircleFilled(ImVec2(x, y), radius, color);
        }
    }

    ImGui::Dummy(ImVec2(progressionWidth, 15.0f));
}

void renderStageSelectPanel(bool includePanel){
    if (includePanel){
        ImGui::Begin("renderStageSelectPanel");
    }

    static int selectedWorld = 0;
    static int selectedLevel = 0;

    auto worlds = worldsForPlaylist("levelselect");
    auto& playlist = *playlistByName("levelselect").value();

    auto activeWorld = worlds.at(selectedWorld);
    auto worldLevels = levelsForWorld(playlist, activeWorld);

    std::cout << "worlds: " << print(worlds) << ", name = " << activeWorld << std::endl;


    int levelsPerWorld = worldLevels.size();

    std::string selectedLevelName = worldLevels.at(selectedLevel) -> level;
    auto levelData = levelByShortcutName(selectedLevelName);

    const float worldWidth = 300.0f;
    const float worldHeight = 70.0f;
    const float spacing = 12.0f;
    float availableWidth = ImGui::GetContentRegionAvail().x;
    float cursorX = ImGui::GetCursorPosX();

    // Background image
    if(levelData.has_value()){
      std::string backgroundTexture = levelData.value().image;

      auto textureId = gameapi -> getTextureSamplerId(backgroundTexture).value();
      ImVec2 panelMin = ImGui::GetWindowPos();
      ImVec2 panelMax = ImVec2(panelMin.x + ImGui::GetWindowWidth(), panelMin.y + ImGui::GetWindowHeight());
      ImGui::GetWindowDrawList() -> AddImage((ImTextureID)(intptr_t)textureId, panelMin, panelMax, ImVec2(0, 1), ImVec2(1, 0), IM_COL32(255, 255, 255, 255));
    }


    // World select
    {
      float selectorWidth = 40.0f + spacing + worldWidth + spacing + 40.0f;

      ImGui::SetCursorPosX(cursorX + (availableWidth - selectorWidth) * 0.5f);
      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 1, 0.12f));
      ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1, 1, 1, 0.20f));
      ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

      if (ImGui::Button("<", ImVec2(40, 40))){
          selectedWorld--;
          if (selectedWorld < 0){
              selectedWorld = 0;
          }
          selectedLevel = 0;
      }

      ImGui::SameLine(0.0f, spacing);
      std::string worldText = activeWorld;
      ImGui::Button(worldText.c_str(), ImVec2(worldWidth, worldHeight));
      ImGui::SameLine(0.0f, spacing);

      if (ImGui::Button(">", ImVec2(40, 40))){
          selectedWorld++;
          if (selectedWorld >= worlds.size()){
              selectedWorld = worlds.size() - 1;
          }
          selectedLevel = 0;
      }
      ImGui::PopStyleVar();
      ImGui::PopStyleColor(3);
    }

    // Level Select
    {
      float levelNavigationWidth = 40.0f + spacing + 180.0f + spacing + 40.0f;
      ImGui::SetCursorPosX(cursorX + (availableWidth - levelNavigationWidth) * 0.5f);
      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 1, 0.12f));
      ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1, 1, 1, 0.20f));

      if (ImGui::Button("-<", ImVec2(40, 40))){
          selectedLevel--;
          if (selectedLevel < 0){
              selectedLevel = levelsPerWorld - 1;
          }
      }

      ImGui::SameLine(0.0f, spacing);
      std::string levelNumber = selectedLevelName;
      ImGui::Button(levelNumber.c_str(), ImVec2(180, 40));
      ImGui::SameLine(0.0f, spacing);

      if (ImGui::Button(">-", ImVec2(40, 40))){
          selectedLevel++;
          if (selectedLevel >= levelsPerWorld){
              selectedLevel = 0;
          }
      }

      ImGui::PopStyleColor(3);
    }

    // --------------------------------------------------
    // Level progression
    // --------------------------------------------------

    //addLevelDots(cursorX, availableWidth, levelsPerWorld, selectedLevel);


    // Level information
    if(levelData.has_value()){
      ImGui::Spacing();

      std::string description = levelData.value().description;
      float descriptionWidth = ImGui::CalcTextSize(description.c_str()).x;
      ImGui::SetCursorPosX(cursorX + (availableWidth - descriptionWidth) * 0.5f);
      ImGui::TextUnformatted(description.c_str());


      ImGui::Spacing();
      ImGui::Spacing();

      float statsWidth = 300.0f;
      float statsX = cursorX + (availableWidth - statsWidth) * 0.5f;

      ImGui::SetCursorPosX(statsX);
      ImGui::Text("BEST TIME");
      ImGui::SameLine(statsX + 190.0f);
      ImGui::Text("02:14.38");

      ImGui::SetCursorPosX(statsX);
      ImGui::Text("HIGH SCORE");
      ImGui::SameLine(statsX + 190.0f);
      ImGui::Text("18,420");

      ImGui::SetCursorPosX(statsX);
      ImGui::Text("SOULS");
      ImGui::SameLine(statsX + 190.0f);
      ImGui::Text("100%%");
    }else{
      ImGui::Text("Missing Level");
    }

    // Play Button
    if(levelData.has_value()){
      ImGui::Spacing();
      ImGui::Spacing();

      const float playWidth = 220.0f;
      const float playHeight = 55.0f;

      ImGui::SetCursorPosX(cursorX + (availableWidth - playWidth) * 0.5f);

      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.15f, 0.90f));
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.30f, 0.30f, 0.95f));
      ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.40f, 0.40f, 0.40f, 1.0f));
      ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

      if (ImGui::Button("PLAY", ImVec2(playWidth, playHeight))){
          // Start selected level here
         goToLevel(selectedLevelName);

      }

      ImGui::PopStyleVar();
      ImGui::PopStyleColor(3);
    }

    if (includePanel){
        ImGui::End();
    }
}