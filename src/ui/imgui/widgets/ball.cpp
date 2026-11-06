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
  
  if (ballModeUi.ballMode.showSoulCount){
    ImGui::Text("Delivered Souls: %d", ballModeUi.ballMode.deliveredSouls);
    ImGui::Text("Lost Souls: %d", ballModeUi.ballMode.lostSouls);

    if (ballModeUi.ballMode.showElapsedTime && ballModeUi.ballMode.elapsedTime.has_value()){
      ImGui::PopFont();

      std::string text = std::to_string(ballModeUi.ballMode.elapsedTime.value()());
      float width = ImGui::CalcTextSize(text.c_str()).x;
      ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - width) * 0.5f);
      ImGui::Text(text.c_str());

      ImGui::PushFont(font);

    }
  }

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
    auto playlistLevel = levelInPlaylist(playlist, selectedLevelName);


    bool isLevelLocked = false;
    if (playlistLevel.value() -> mustUnlock){
      isLevelLocked = true;
      if (selectedLevel == 0){
        isLevelLocked = false;
      }else{
        auto previousLevel = worldLevels.at(selectedLevel - 1);
        isLevelLocked = !isLevelComplete(previousLevel -> level);
      }
    }


    if (!levelData.has_value() || !playlistLevel.has_value()){
      levelData = std::nullopt;
      playlistLevel = std::nullopt;
    }

    const float worldWidth = 300.0f;
    const float worldHeight = 70.0f;
    const float spacing = 12.0f;
    float availableWidth = ImGui::GetContentRegionAvail().x;
    float cursorX = ImGui::GetCursorPosX();

    // Background image
    if(levelData.has_value()){
      if (isLevelLocked){
        ImGui::Text("Level Locked");
      }else{
        std::string backgroundTexture = levelData.value().image;
        auto textureId = gameapi -> getTextureSamplerId(backgroundTexture).value();
        ImVec2 panelMin = ImGui::GetWindowPos();
        ImVec2 panelMax = ImVec2(panelMin.x + ImGui::GetWindowWidth(), panelMin.y + ImGui::GetWindowHeight());
        ImGui::GetWindowDrawList() -> AddImage((ImTextureID)(intptr_t)textureId, panelMin, panelMax, ImVec2(0, 1), ImVec2(1, 0), IM_COL32(255, 255, 255, 255));        
      }

    }


    // World select
    {
      float selectorWidth = 40.0f + spacing + worldWidth + spacing + 40.0f;

      ImGui::SetCursorPosX(cursorX + (availableWidth - selectorWidth) * 0.5f);
      ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 1, 0.12f));
      ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1, 1, 1, 0.20f));
      ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

      auto oldWorld = selectedWorld;
      if (ImGui::Button("<", ImVec2(40, 40))){
          selectedWorld--;
          if (selectedWorld < 0){
              selectedWorld = 0;
          }
          if (selectedWorld != oldWorld){
            selectedLevel = 0;
          }
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
          if (selectedWorld != oldWorld){
            selectedLevel = 0;
          }
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
              selectedLevel = 0;
          }
      }

      ImGui::SameLine(0.0f, spacing);
      std::string levelNumber = selectedLevelName;
      ImGui::Button(levelNumber.c_str(), ImVec2(180, 40));
      ImGui::SameLine(0.0f, spacing);

      if (ImGui::Button(">-", ImVec2(40, 40))){
          selectedLevel++;
          if (selectedLevel >= levelsPerWorld){
              selectedLevel = (levelsPerWorld - 1);
          }
      }

      ImGui::PopStyleColor(3);
    }

    // --------------------------------------------------
    // Level progression
    // --------------------------------------------------

    //addLevelDots(cursorX, availableWidth, levelsPerWorld, selectedLevel);


    // Level information
    if(!levelData.has_value()){
      ImGui::Text("Missing Level");
    }else {
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
      ImGui::Text("PAR TIME");
      ImGui::SameLine(statsX + 190.0f);
      ImGui::Text(printFloat(playlistLevel.value() -> parTime).c_str());

      ImGui::SetCursorPosX(statsX);
      ImGui::Text("HIGH SCORE");
      ImGui::SameLine(statsX + 190.0f);
      ImGui::Text("18,420");

      ImGui::SetCursorPosX(statsX);
      ImGui::Text("SOULS");
      ImGui::SameLine(statsX + 190.0f);
      ImGui::Text("100%%");
    }

    // Play Button
    if(!isLevelLocked && levelData.has_value()){
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



struct RhythmNoteUi {
  int lane;
  float y;
  ImU32 color;
};


void renderRhythmPanel(bool includePanel){
    if (includePanel){
        ImGui::Begin("renderRhythmPanel");
    }

    auto notesToRenderOpt = getRhythmRendering(10);
    if (!notesToRenderOpt.has_value()){
      if (includePanel){
        ImGui::End();
      }
      return;
    }

    auto& notesToRender = notesToRenderOpt.value();

    modassert(notesToRender.rawSongData != NULL, "notes to render is null");

    ImGui::Text("Display Name: %s", notesToRender.rawSongData -> displayName.c_str());
    ImGui::Text("Num Notes: %d", notesToRender.stats.numNotesTotal);
    ImGui::Text("Num Success: %d", notesToRender.stats.numSuccess);
    ImGui::Text("Num Failure: %d", notesToRender.stats.numFailure);
    ImGui::Text("Complete: %s", notesToRender.stats.songComplete ? "true" : "false");
    ImGui::Text("Playback Complete: %s", notesToRender.stats.songPlaybackComplete ? "true" : "false");
    ImGui::Text("Health: %i / %i", notesToRender.stats.songCurrentHealth, notesToRender.stats.songTotalHealth);


    auto drawList = ImGui::GetWindowDrawList();

    ImVec2 panelMin = ImGui::GetWindowPos();
    ImVec2 panelMax = ImVec2(panelMin.x + ImGui::GetWindowWidth(), panelMin.y + ImGui::GetWindowHeight());

    // Rhythm chart area
    float chartWidth = 300.0f;
    float chartHeight = 500.0f;

    ImVec2 chartMin = ImVec2(
        panelMin.x + (ImGui::GetWindowWidth() - chartWidth) * 0.5f,
        panelMin.y + 80.0f
    );

    ImVec2 chartMax = ImVec2(
        chartMin.x + chartWidth,
        chartMin.y + chartHeight
    );

    // Background image

    std::string backgroundTexture = "./res/textures/wood.jpg";
    auto textureId = gameapi -> getTextureSamplerId(backgroundTexture).value();

    float imageTransparency = 0.2f;

    // Top
    ImGui::PushClipRect(panelMin, ImVec2(panelMax.x, chartMin.y), true);

    drawList -> AddImage(
        (ImTextureID)(intptr_t)textureId,
        panelMin,
        panelMax,
        ImVec2(0, 1),
        ImVec2(1, 0),
        IM_COL32(255, 255, 255, (int)(255 * imageTransparency))
    );

    ImGui::PopClipRect();

    ImGui::PushClipRect(ImVec2(panelMin.x, chartMax.y), panelMax, true);

    drawList -> AddImage(
        (ImTextureID)(intptr_t)textureId,
        panelMin,
        panelMax,
        ImVec2(0, 1),
        ImVec2(1, 0),
        IM_COL32(255, 255, 255, (int)(255 * imageTransparency))
    );

    ImGui::PopClipRect();
    ImGui::PushClipRect(ImVec2(panelMin.x, chartMin.y), ImVec2(chartMin.x, chartMax.y), true);

    drawList -> AddImage(
        (ImTextureID)(intptr_t)textureId,
        panelMin,
        panelMax,
        ImVec2(0, 1),
        ImVec2(1, 0),
        IM_COL32(255, 255, 255, (int)(255 * imageTransparency))
    );

    ImGui::PopClipRect();

    // Right
    ImGui::PushClipRect(ImVec2(chartMax.x, chartMin.y), ImVec2(panelMax.x, chartMax.y), true);

    drawList -> AddImage(
        (ImTextureID)(intptr_t)textureId,
        panelMin,
        panelMax,
        ImVec2(0, 1),
        ImVec2(1, 0),
        IM_COL32(255, 255, 255, (int)(255 * imageTransparency))
    );

    ImGui::PopClipRect();


    // Semi-transparent black rhythm chart
    drawList -> AddRectFilled(chartMin, chartMax, IM_COL32(0, 0, 0, (int)(255 * 0.4f)));


    // Chart border
    drawList -> AddRect(
        chartMin,
        chartMax,
        IM_COL32(255, 255, 255, 100),
        0.0f,
        0,
        2.0f
    );


    // Three rhythm lanes
    float laneWidth = chartWidth / 3.0f;

    for (int i = 1; i < 3; i++){
        float x = chartMin.x + laneWidth * i;
        drawList -> AddLine(ImVec2(x, chartMin.y), ImVec2(x, chartMax.y), IM_COL32(255, 255, 255, 40), 1.0f);
    }


    // Hit line
    float hitY = chartMax.y - 70.0f;

    drawList -> AddLine(ImVec2(chartMin.x, hitY), ImVec2(chartMax.x, hitY), IM_COL32(255, 255, 255, 180), 2.0f);

    {
      // Notes
      float noteWidth = laneWidth * 0.55f;
      float noteHeight = 24.0f;

      float noteTransparency = 0.5f;

      std::vector<RhythmNoteUi> notes {}; 


      int pixelsPerSecond = 100;
      auto timeOffset = notesToRender.currentPlaybackTime * pixelsPerSecond;

      for (auto& note : notesToRender.notes){
        notes.push_back(RhythmNoteUi {
          .lane = note.lane,
          .y = chartMax.y - note.time * pixelsPerSecond + timeOffset,
          .color = IM_COL32(note.color.r * 255, note.color.g * 255, note.color.b * 255, 255 * note.color.a),
        });
      }     

      notes.push_back(RhythmNoteUi {
        .lane = 0,
        .y = chartMax.y ,
        .color = IM_COL32(notesToRender.lane0Pressed ? 255 : 0, 0, 0, 255),
      });
      notes.push_back(RhythmNoteUi {
        .lane = 1,
        .y = chartMax.y ,
        .color = IM_COL32(notesToRender.lane1Pressed ? 255 : 0, 0, 0, 255),
      });
      notes.push_back(RhythmNoteUi {
        .lane = 2,
        .y = chartMax.y,
        .color = IM_COL32(notesToRender.lane2Pressed ? 255 : 0, 0, 0, 255),
      });



      for (auto& note : notes){
          float centerX = chartMin.x + laneWidth * note.lane  + laneWidth * 0.5f;
          ImVec2 noteMin = ImVec2(centerX - noteWidth * 0.5f, note.y - noteHeight * 0.5f);
          ImVec2 noteMax = ImVec2(centerX + noteWidth * 0.5f, note.y + noteHeight * 0.5f);
          drawList -> AddRectFilled(noteMin, noteMax, note.color, 5.0f);
          drawList -> AddRect(noteMin, noteMax, IM_COL32(255, 255, 255, 255), 5.0f, 0, 1.0f);
      }
    }


    if (includePanel){
        ImGui::End();
    }
}