#include "./ball.h"
#include <algorithm>

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

    auto uiOffset = getActiveControllable(getDefaultPlayerIndex()).value() -> uiShakeOffset;
    auto uiMoveOffset = getActiveControllable(getDefaultPlayerIndex()).value() -> uiMoveOffset;
    uiOffset = uiOffset + glm::vec2(uiMoveOffset.x,  -uiMoveOffset.y);

    ImGui::SetCursorPosX(uiOffset.x * -100);
    ImGui::SetCursorPosY(uiOffset.y * -50 + 100);

    ImGui::Text("Delivered Souls: %d", ballModeUi.ballMode.deliveredSouls);

    ImGui::SetCursorPosX(uiOffset.x * -100);
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
    static float wheelPosition = 0.0f;

    auto worlds = worldsForPlaylist("levelselect");
    auto playlistOpt = playlistByName("levelselect");
    if (worlds.empty() || !playlistOpt.has_value()){
      ImGui::TextUnformatted("No stages available");
      if (includePanel){
        ImGui::End();
      }
      return;
    }

    selectedWorld = std::clamp(selectedWorld, 0, (int)worlds.size() - 1);

    ImVec2 origin = ImGui::GetCursorScreenPos();
    ImVec2 available = ImGui::GetContentRegionAvail();
    const float margin = 20.0f;
    const float gap = 20.0f;
    const float contentWidth = available.x - margin * 2.0f;
    const float bodyHeight = available.y - 62.0f;
    const float leftWidth = (contentWidth - gap) * 0.62f;
    const float rightWidth = contentWidth - gap - leftWidth;
    const float leftX = origin.x + margin;
    const float rightX = leftX + leftWidth + gap;
    const float bodyY = origin.y + 58.0f;
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    auto oldWorld = selectedWorld;
    ImGui::SetCursorScreenPos(ImVec2(leftX, origin.y));
    if (ImGui::Button("<", ImVec2(42.0f, 42.0f))){
      selectedWorld = std::max(0, selectedWorld - 1);
    }
    ImGui::SetCursorScreenPos(ImVec2(leftX + 54.0f, origin.y));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.10f, 0.15f, 0.85f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.14f, 0.20f, 0.29f, 0.95f));
    ImGui::Button(worlds.at(selectedWorld).c_str(), ImVec2(leftWidth - 108.0f, 42.0f));
    ImGui::PopStyleColor(2);
    ImGui::SetCursorScreenPos(ImVec2(leftX + leftWidth - 42.0f, origin.y));
    if (ImGui::Button(">", ImVec2(42.0f, 42.0f))){
      selectedWorld = std::min((int)worlds.size() - 1, selectedWorld + 1);
    }
    if (selectedWorld != oldWorld){
      selectedLevel = 0;
      wheelPosition = 0.0f;
    }

    auto& playlist = *playlistOpt.value();
    auto worldLevels = levelsForWorld(playlist, worlds.at(selectedWorld));
    if (worldLevels.empty()){
      ImGui::SetCursorScreenPos(ImVec2(leftX, bodyY));
      ImGui::TextUnformatted("No stages in this world");
      if (includePanel){
        ImGui::End();
      }
      return;
    }

    selectedLevel = std::clamp(selectedLevel, 0, (int)worldLevels.size() - 1);
    if (wheelPosition > (float)worldLevels.size() - 1.0f){
      wheelPosition = (float)worldLevels.size() - 1.0f;
    }

    if (ImGui::IsMouseHoveringRect(ImVec2(rightX, bodyY), ImVec2(rightX + rightWidth, bodyY + bodyHeight))){
      float wheel = ImGui::GetIO().MouseWheel;
      if (wheel != 0.0f){
        selectedLevel = std::clamp(selectedLevel - (int)wheel, 0, (int)worldLevels.size() - 1);
      }
    }
    if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)){
      selectedLevel = std::max(0, selectedLevel - 1);
    }
    if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)){
      selectedLevel = std::min((int)worldLevels.size() - 1, selectedLevel + 1);
    }

    wheelPosition += (selectedLevel - wheelPosition) * std::min(1.0f, ImGui::GetIO().DeltaTime * 12.0f);

    std::string selectedLevelName = worldLevels.at(selectedLevel)->level;
    auto levelData = levelByShortcutName(selectedLevelName);
    auto isLocked = [&worldLevels](int index) {
      if (!worldLevels.at(index)->mustUnlock || index == 0){
        return false;
      }
      return !isLevelComplete(worldLevels.at(index - 1)->level);
    };
    bool isLevelLocked = isLocked(selectedLevel);
    float imageHeight = bodyHeight * 0.48f;

    drawList->AddRectFilled(ImVec2(leftX, bodyY), ImVec2(leftX + leftWidth, bodyY + bodyHeight), IM_COL32(9, 14, 23, 235), 8.0f);
    drawList->AddRect(ImVec2(leftX, bodyY), ImVec2(leftX + leftWidth, bodyY + bodyHeight), IM_COL32(95, 131, 171, 170), 8.0f, 0, 1.5f);
    ImVec2 imageMin(leftX + 1.0f, bodyY + 1.0f);
    ImVec2 imageMax(leftX + leftWidth - 1.0f, bodyY + imageHeight);
    drawList->AddRectFilled(imageMin, imageMax, IM_COL32(20, 29, 42, 255), 7.0f);
    if (levelData.has_value() && !levelData->image.empty()){
      auto textureId = gameapi->getTextureSamplerId(levelData->image);
      if (textureId.has_value()){
        drawList->AddImage((ImTextureID)(intptr_t)textureId.value(), imageMin, imageMax, ImVec2(0, 1), ImVec2(1, 0));
      }
    }
    drawList->AddRectFilled(imageMin, ImVec2(imageMax.x, imageMax.y), IM_COL32(0, 0, 0, 35), 7.0f);

    std::string stageTitle = levelData.has_value() ? levelData->name : selectedLevelName;
    drawList->AddText(ImVec2(leftX + 18.0f, bodyY + imageHeight + 14.0f), IM_COL32(245, 249, 255, 255), stageTitle.c_str());
    if (!levelData.has_value()){
      drawList->AddText(ImVec2(leftX + 18.0f, bodyY + imageHeight + 42.0f), IM_COL32(220, 225, 235, 220), "Missing stage data");
    }else{
      ImVec2 descriptionMin(leftX + 18.0f, bodyY + imageHeight + 44.0f);
      ImVec2 descriptionMax(leftX + leftWidth - 18.0f, bodyY + bodyHeight - 106.0f);
      drawList->PushClipRect(descriptionMin, descriptionMax, true);
      drawList->AddText(ImGui::GetFont(), ImGui::GetFontSize(), descriptionMin, IM_COL32(205, 215, 228, 230), levelData->description.c_str(), nullptr, leftWidth - 36.0f);
      drawList->PopClipRect();

      auto progress = getLevelProgressInfo(worlds.at(selectedWorld), selectedLevelName);
      float statsY = bodyY + bodyHeight - 91.0f;
      drawList->AddText(ImVec2(leftX + 18.0f, statsY), IM_COL32(120, 157, 202, 255), "PAR");
      std::string parTime = printFloat(progress.parTime);
      drawList->AddText(ImVec2(leftX + 18.0f, statsY + 20.0f), IM_COL32(240, 244, 250, 255), parTime.c_str());
      drawList->AddText(ImVec2(leftX + leftWidth * 0.34f, statsY), IM_COL32(120, 157, 202, 255), "BEST");
      std::string bestTime = progress.bestTime.has_value() ? printFloat(progress.bestTime.value()) : "--";
      drawList->AddText(ImVec2(leftX + leftWidth * 0.34f, statsY + 20.0f), IM_COL32(240, 244, 250, 255), bestTime.c_str());
      drawList->AddText(ImVec2(leftX + leftWidth * 0.62f, statsY), IM_COL32(120, 157, 202, 255), "CRYSTALS");
      std::string crystals = std::to_string(progress.gemCount) + " / " + std::to_string(progress.totalGemCount);
      drawList->AddText(ImVec2(leftX + leftWidth * 0.62f, statsY + 20.0f), IM_COL32(240, 244, 250, 255), crystals.c_str());
    }

    if (isLevelLocked){
      drawList->AddRectFilled(imageMin, imageMax, IM_COL32(0, 0, 0, 145), 7.0f);
      drawList->AddText(ImVec2(leftX + 18.0f, bodyY + imageHeight - 38.0f), IM_COL32(255, 211, 105, 255), "LOCKED");
    }

    ImGui::SetCursorScreenPos(ImVec2(leftX + leftWidth - 190.0f, bodyY + bodyHeight - 58.0f));
    ImGui::BeginDisabled(isLevelLocked || !levelData.has_value());
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.10f, 0.46f, 0.82f, 0.95f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.16f, 0.58f, 0.98f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.07f, 0.36f, 0.68f, 1.0f));
    if (ImGui::Button("PLAY STAGE", ImVec2(172.0f, 44.0f))){
      goToLevel(selectedLevelName);
    }
    ImGui::PopStyleColor(3);
    ImGui::EndDisabled();

    ImVec2 wheelMin(rightX, bodyY);
    ImVec2 wheelMax(rightX + rightWidth, bodyY + bodyHeight);
    drawList->AddRectFilled(wheelMin, wheelMax, IM_COL32(7, 11, 18, 230), 8.0f);
    drawList->AddRect(wheelMin, wheelMax, IM_COL32(95, 131, 171, 170), 8.0f, 0, 1.5f);
    drawList->AddText(ImVec2(rightX + 18.0f, bodyY + 15.0f), IM_COL32(120, 157, 202, 255), "STAGE SELECT");
    drawList->AddText(ImVec2(rightX + rightWidth - 82.0f, bodyY + 15.0f), IM_COL32(120, 157, 202, 190), "UP / DOWN");

    const float rowHeight = std::min(68.0f, (bodyHeight - 56.0f) / 5.0f);
    const float wheelCenterY = bodyY + 48.0f + (bodyHeight - 48.0f) * 0.5f;
    drawList->PushClipRect(ImVec2(rightX + 2.0f, bodyY + 42.0f), ImVec2(rightX + rightWidth - 2.0f, bodyY + bodyHeight - 5.0f), true);
    int firstLevel = std::max(0, (int)wheelPosition - 3);
    int lastLevel = std::min((int)worldLevels.size() - 1, (int)wheelPosition + 3);
    for (int index = firstLevel; index <= lastLevel; index++){
      float offset = index - wheelPosition;
      float rowY = wheelCenterY + offset * rowHeight - rowHeight * 0.5f;
      ImVec2 rowMin(rightX + 12.0f, rowY);
      ImVec2 rowSize(rightWidth - 24.0f, rowHeight - 7.0f);
      ImGui::SetCursorScreenPos(rowMin);
      ImGui::PushID(index);
      bool clicked = ImGui::InvisibleButton("stage-wheel-row", rowSize);
      bool hovered = ImGui::IsItemHovered();
      if (clicked){
        selectedLevel = index;
      }

      bool selected = index == selectedLevel;
      ImU32 rowColor = selected ? IM_COL32(28, 79, 125, 245) : hovered ? IM_COL32(30, 43, 61, 220) : IM_COL32(17, 25, 37, 190);
      ImU32 borderColor = selected ? IM_COL32(91, 187, 255, 255) : IM_COL32(67, 87, 111, 130);
      drawList->AddRectFilled(rowMin, ImVec2(rowMin.x + rowSize.x, rowMin.y + rowSize.y), rowColor, 5.0f);
      drawList->AddRect(rowMin, ImVec2(rowMin.x + rowSize.x, rowMin.y + rowSize.y), borderColor, 5.0f, 0, selected ? 2.0f : 1.0f);

      std::string rowTitle = worldLevels.at(index)->level;
      auto rowLevelData = levelByShortcutName(rowTitle);
      if (rowLevelData.has_value()){
        rowTitle = rowLevelData->name;
      }
      ImU32 textColor = isLocked(index) ? IM_COL32(145, 153, 165, 190) : IM_COL32(231, 238, 247, selected ? 255 : 205);
      drawList->AddText(ImVec2(rowMin.x + 14.0f, rowMin.y + (rowSize.y - ImGui::GetFontSize()) * 0.5f), textColor, rowTitle.c_str());
      if (isLocked(index)){
        drawList->AddText(ImVec2(rowMin.x + rowSize.x - 60.0f, rowMin.y + (rowSize.y - ImGui::GetFontSize()) * 0.5f), IM_COL32(230, 184, 96, 220), "LOCKED");
      }
      ImGui::PopID();
    }
    drawList->PopClipRect();

    ImGui::SetCursorScreenPos(ImVec2(origin.x, origin.y + available.y));
    ImGui::Dummy(ImVec2(available.x, 1.0f));

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