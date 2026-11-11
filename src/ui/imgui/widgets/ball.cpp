#include "./ball.h"
#include <algorithm>
#include <cmath>
#include <unordered_map>

extern CustomApiBindings* gameapi;

void reloadVehicleSettings();
void goToLevel(std::string levelShortName, std::optional<std::any> hint, bool forceReload);

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
    ImDrawListSplitter drawListSplitter;
    drawListSplitter.Split(drawList, 2);
    drawListSplitter.SetCurrentChannel(drawList, 1);

    auto oldWorld = selectedWorld;
    drawList->AddRectFilled(
      ImVec2(leftX, origin.y),
      ImVec2(leftX + leftWidth, origin.y + 42.0f),
      IM_COL32(10, 17, 28, 225),
      7.0f
    );
    drawList->AddRect(
      ImVec2(leftX, origin.y),
      ImVec2(leftX + leftWidth, origin.y + 42.0f),
      IM_COL32(55, 86, 112, 150),
      7.0f
    );
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 6.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.055f, 0.095f, 0.15f, 0.96f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.08f, 0.24f, 0.31f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.05f, 0.34f, 0.4f, 1.0f));
    ImGui::SetCursorScreenPos(ImVec2(leftX, origin.y));
    if (ImGui::Button("<", ImVec2(42.0f, 42.0f))){
      selectedWorld = std::max(0, selectedWorld - 1);
    }
    ImGui::SetCursorScreenPos(ImVec2(leftX + 54.0f, origin.y));
    ImGui::Button(worlds.at(selectedWorld).c_str(), ImVec2(leftWidth - 108.0f, 42.0f));
    ImGui::SetCursorScreenPos(ImVec2(leftX + leftWidth - 42.0f, origin.y));
    if (ImGui::Button(">", ImVec2(42.0f, 42.0f))){
      selectedWorld = std::min((int)worlds.size() - 1, selectedWorld + 1);
    }
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);
    if (selectedWorld != oldWorld){
      selectedLevel = 0;
      wheelPosition = 0.0f;
    }

    auto& playlist = *playlistOpt.value();
    auto worldLevels = levelsForWorld(playlist, worlds.at(selectedWorld));
    if (worldLevels.empty()){
      ImGui::SetCursorScreenPos(ImVec2(leftX, bodyY));
      ImGui::TextUnformatted("No stages in this world");
      drawListSplitter.Merge(drawList);
      if (includePanel){
        ImGui::End();
      }
      return;
    }

    const int levelCount = (int)worldLevels.size();
    selectedLevel = std::clamp(selectedLevel, 0, levelCount - 1);

    if (ImGui::IsMouseHoveringRect(ImVec2(rightX, bodyY), ImVec2(rightX + rightWidth, bodyY + bodyHeight))){
      float wheel = ImGui::GetIO().MouseWheel;
      if (wheel != 0.0f){
        selectedLevel = std::clamp(selectedLevel - (int)wheel, 0, levelCount - 1);
      }
    }
    if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)){
      selectedLevel = std::max(0, selectedLevel - 1);
    }
    if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)){
      selectedLevel = std::min(levelCount - 1, selectedLevel + 1);
    }

    wheelPosition += (selectedLevel - wheelPosition) * std::min(1.0f, ImGui::GetIO().DeltaTime * 18.0f);

    std::string selectedLevelName = worldLevels.at(selectedLevel)->level;
    auto levelData = levelByShortcutName(selectedLevelName);
    static std::unordered_map<std::string, float> imageAspectCache;
    auto isLocked = [&worldLevels](int index) {
      if (getArgEnabled("unlock")){
        return false;
      }
      if (!worldLevels.at(index)->mustUnlock || index == 0){
        return false;
      }
      return !isLevelComplete(worldLevels.at(index - 1)->level);
    };
    bool isLevelLocked = isLocked(selectedLevel);
    float imageHeight = bodyHeight * 0.48f;
    ImVec2 imageMin(leftX + 1.0f, bodyY + 1.0f);
    ImVec2 imageMax(leftX + leftWidth - 1.0f, bodyY + imageHeight);
    auto drawCoverTexture = [&](std::string texturePath, ImVec2 targetMin, ImVec2 targetMax, ImU32 tint, ImDrawList* targetDrawList){
      auto textureId = gameapi->getTextureSamplerId(texturePath);
      if (!textureId.has_value()){
        return;
      }

      auto aspect = imageAspectCache.find(texturePath);
      if (aspect == imageAspectCache.end()){
        auto dimensions = gameapi->getTextureSize(texturePath);
        if (dimensions.x <= 0.0f || dimensions.y <= 0.0f){
          return;
        }
        aspect = imageAspectCache.emplace(texturePath, dimensions.x / dimensions.y).first;
      }

      float textureAspect = aspect->second;
      float frameWidth = targetMax.x - targetMin.x;
      float frameHeight = targetMax.y - targetMin.y;
      if (frameWidth <= 0.0f || frameHeight <= 0.0f){
        return;
      }
      float frameAspect = frameWidth / frameHeight;
      ImVec2 uvMin(0.0f, 1.0f);
      ImVec2 uvMax(1.0f, 0.0f);
      if (textureAspect > frameAspect){
        float visibleWidth = frameAspect / textureAspect;
        uvMin.x = (1.0f - visibleWidth) * 0.5f;
        uvMax.x = 1.0f - uvMin.x;
      }else{
        float visibleHeight = textureAspect / frameAspect;
        uvMin.y = 0.5f + visibleHeight * 0.5f;
        uvMax.y = 0.5f - visibleHeight * 0.5f;
      }
      targetDrawList->AddImage((ImTextureID)(intptr_t)textureId.value(), targetMin, targetMax, uvMin, uvMax, tint);
    };

    bool hasBackgroundTexture = levelData.has_value() && levelData->backgroundTexture.has_value();
    drawList->AddRectFilledMultiColor(
      ImVec2(leftX, bodyY),
      ImVec2(leftX + leftWidth, bodyY + bodyHeight),
      IM_COL32(13, 23, 36, 225),
      IM_COL32(8, 14, 24, 225),
      IM_COL32(6, 11, 19, 225),
      IM_COL32(10, 18, 30, 225)
    );
    drawList->AddRect(ImVec2(leftX, bodyY), ImVec2(leftX + leftWidth, bodyY + bodyHeight), IM_COL32(65, 103, 133, 190), 8.0f, 0, 1.5f);
    drawList->AddRectFilled(ImVec2(leftX + 18.0f, bodyY + 9.0f), ImVec2(leftX + 70.0f, bodyY + 12.0f), IM_COL32(57, 224, 222, 220), 2.0f);
    drawList->AddRectFilled(imageMin, imageMax, IM_COL32(16, 29, 43, 255), 7.0f);
    if (levelData.has_value() && !levelData->image.empty()){
      drawCoverTexture(levelData->image, imageMin, imageMax, IM_COL32(255, 255, 255, 255), drawList);
    }
    drawList->AddRectFilled(imageMin, ImVec2(imageMax.x, imageMax.y), IM_COL32(0, 0, 0, 35), 7.0f);
    drawList->AddRectFilledMultiColor(
      ImVec2(imageMin.x, imageMax.y - 54.0f),
      imageMax,
      IM_COL32(5, 11, 19, 0),
      IM_COL32(5, 11, 19, 0),
      IM_COL32(5, 11, 19, 150),
      IM_COL32(5, 11, 19, 150)
    );
    if (levelData.has_value() && levelData->secondaryBackgroundTexture.has_value()){
      ImVec2 detailsMin(leftX + 2.0f, imageMax.y + 2.0f);
      ImVec2 detailsMax(leftX + leftWidth - 2.0f, bodyY + bodyHeight - 2.0f);
      drawCoverTexture(levelData->secondaryBackgroundTexture.value(), detailsMin, detailsMax, IM_COL32(255, 255, 255, 255), drawList);
    }

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
      drawList->AddLine(ImVec2(leftX + 18.0f, statsY - 9.0f), ImVec2(leftX + leftWidth - 18.0f, statsY - 9.0f), IM_COL32(74, 108, 132, 120));
      drawList->AddText(ImVec2(leftX + 18.0f, statsY), IM_COL32(91, 190, 199, 255), "PAR");
      std::string parTime = printFloat(progress.parTime);
      drawList->AddText(ImVec2(leftX + 18.0f, statsY + 20.0f), IM_COL32(240, 244, 250, 255), parTime.c_str());
      drawList->AddText(ImVec2(leftX + leftWidth * 0.34f, statsY), IM_COL32(91, 190, 199, 255), "BEST");
      std::string bestTime = progress.bestTime.has_value() ? printFloat(progress.bestTime.value()) : "--";
      drawList->AddText(ImVec2(leftX + leftWidth * 0.34f, statsY + 20.0f), IM_COL32(240, 244, 250, 255), bestTime.c_str());
      drawList->AddText(ImVec2(leftX + leftWidth * 0.62f, statsY), IM_COL32(91, 190, 199, 255), "CRYSTALS");
      std::string crystals = std::to_string(progress.gemCount) + " / " + std::to_string(progress.totalGemCount);
      drawList->AddText(ImVec2(leftX + leftWidth * 0.62f, statsY + 20.0f), IM_COL32(240, 244, 250, 255), crystals.c_str());
    }

    if (isLevelLocked){
      drawList->AddRectFilled(imageMin, imageMax, IM_COL32(0, 0, 0, 145), 7.0f);
      drawList->AddText(ImVec2(leftX + 18.0f, bodyY + imageHeight - 38.0f), IM_COL32(255, 198, 104, 255), "LOCKED");
    }

    ImGui::SetCursorScreenPos(ImVec2(leftX + leftWidth - 190.0f, bodyY + bodyHeight - 58.0f));
    ImGui::BeginDisabled(isLevelLocked || !levelData.has_value());
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.04f, 0.48f, 0.51f, 0.98f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.06f, 0.67f, 0.67f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.03f, 0.37f, 0.42f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 7.0f);
    bool playClicked = ImGui::Button("PLAY STAGE", ImVec2(172.0f, 44.0f));
    if (!isLevelLocked && levelData.has_value() && (playClicked || ImGui::IsKeyPressed(ImGuiKey_Enter))){
      goToLevel(
        selectedLevelName, 
        LevelLoadOptions {
          .arcadeMode = true,
        }, 
        false
      );
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);
    ImGui::EndDisabled();

    ImVec2 wheelMin(rightX, bodyY);
    ImVec2 wheelMax(rightX + rightWidth, bodyY + bodyHeight);
    if (hasBackgroundTexture){
      drawListSplitter.SetCurrentChannel(drawList, 0);
      drawList->PushClipRect(wheelMin, wheelMax, false);
      drawCoverTexture(levelData->backgroundTexture.value(), wheelMin, wheelMax, IM_COL32(255, 255, 255, 255), drawList);
      drawList->PopClipRect();
      drawListSplitter.SetCurrentChannel(drawList, 1);
    }
    int wheelPanelAlpha = hasBackgroundTexture ? 0 : 225;
    drawList->AddRectFilledMultiColor(
      wheelMin,
      wheelMax,
      IM_COL32(11, 20, 32, wheelPanelAlpha),
      IM_COL32(7, 13, 22, wheelPanelAlpha),
      IM_COL32(6, 11, 19, wheelPanelAlpha),
      IM_COL32(9, 17, 28, wheelPanelAlpha)
    );
    drawList->PushClipRect(wheelMin, wheelMax, true);
    for (float x = rightX + 16.0f; x < rightX + rightWidth; x += 32.0f){
      drawList->AddLine(ImVec2(x, bodyY + 43.0f), ImVec2(x, bodyY + bodyHeight), IM_COL32(88, 142, 165, 12));
    }
    drawList->PopClipRect();
    drawList->AddRect(wheelMin, wheelMax, IM_COL32(65, 103, 133, 190), 8.0f, 0, 1.5f);
    drawList->AddRectFilled(ImVec2(rightX + 18.0f, bodyY + 28.0f), ImVec2(rightX + 58.0f, bodyY + 30.0f), IM_COL32(57, 224, 222, 190), 1.0f);
    drawList->AddText(ImVec2(rightX + 18.0f, bodyY + 9.0f), IM_COL32(132, 205, 211, 255), "STAGE SELECT");
    drawList->AddText(ImVec2(rightX + rightWidth - 82.0f, bodyY + 11.0f), IM_COL32(116, 144, 163, 210), "UP / DOWN");

    const float rowHeight = std::min(68.0f, (bodyHeight - 56.0f) / 5.0f);
    const float wheelCenterY = bodyY + 48.0f + (bodyHeight - 48.0f) * 0.5f;
    drawList->PushClipRect(ImVec2(rightX + 2.0f, bodyY + 42.0f), ImVec2(rightX + rightWidth - 2.0f, bodyY + bodyHeight - 5.0f), true);
    int firstLevel = std::max(0, (int)std::floor(wheelPosition) - 3);
    int lastLevel = std::min(levelCount - 1, (int)std::floor(wheelPosition) + 3);
    for (int index = firstLevel; index <= lastLevel; index++){
      float offset = index - wheelPosition;
      float curve = std::min(std::abs(offset) / 3.0f, 1.0f);
      float curveAmount = curve * curve;
      float cardWidth = (rightWidth - 24.0f) * (1.0f - curveAmount * 0.24f);
      float horizontalShift = curveAmount * rightWidth * 0.08f;
      float rowY = wheelCenterY + offset * rowHeight - rowHeight * 0.5f;
      float cardHeight = (rowHeight - 7.0f) * (1.0f - curveAmount * 0.22f);
      float fade = 1.0f - curveAmount * 0.68f;
      ImVec2 rowMin(rightX + 12.0f + horizontalShift + (rightWidth - 24.0f - cardWidth) * 0.5f, rowY + (rowHeight - 7.0f - cardHeight) * 0.5f);
      ImVec2 rowSize(cardWidth, cardHeight);
      ImGui::SetCursorScreenPos(rowMin);
      ImGui::PushID(index);
      bool clicked = ImGui::InvisibleButton("stage-wheel-row", rowSize);
      bool hovered = ImGui::IsItemHovered();
      if (clicked){
        selectedLevel = index;
      }

      bool selected = index == selectedLevel;
      int rowAlpha = (int)((selected ? 245 : hovered ? 220 : 190) * fade);
      int borderAlpha = selected ? 255 : (int)(130 * fade);
      ImU32 rowColor = selected ? IM_COL32(12, 69, 76, rowAlpha) : hovered ? IM_COL32(25, 45, 55, rowAlpha) : IM_COL32(13, 23, 34, rowAlpha);
      ImU32 borderColor = selected ? IM_COL32(68, 225, 213, borderAlpha) : IM_COL32(55, 81, 101, borderAlpha);
      drawList->AddRectFilled(rowMin, ImVec2(rowMin.x + rowSize.x, rowMin.y + rowSize.y), rowColor, 5.0f);
      if (selected){
        drawList->AddRect(ImVec2(rowMin.x - 2.0f, rowMin.y - 2.0f), ImVec2(rowMin.x + rowSize.x + 2.0f, rowMin.y + rowSize.y + 2.0f), IM_COL32(79, 231, 220, 95), 7.0f, 0, 1.5f);
      }
      drawList->AddRect(rowMin, ImVec2(rowMin.x + rowSize.x, rowMin.y + rowSize.y), borderColor, 5.0f, 0, selected ? 2.0f : 1.0f);

      std::string rowTitle = worldLevels.at(index)->level;
      auto rowLevelData = levelByShortcutName(rowTitle);
      if (rowLevelData.has_value()){
        rowTitle = rowLevelData->name;
      }
      std::string stageNumber = std::to_string(index + 1);
      if (index + 1 < 10){
        stageNumber = "0" + stageNumber;
      }
      int textAlpha = (int)((isLocked(index) ? 190 : selected ? 255 : 220) * fade);
      ImU32 textColor = isLocked(index) ? IM_COL32(145, 153, 165, textAlpha) : IM_COL32(231, 238, 247, textAlpha);
      const float thumbnailSize = std::min(42.0f, rowSize.y - 10.0f);
      ImVec2 thumbnailMin(rowMin.x + 9.0f, rowMin.y + (rowSize.y - thumbnailSize) * 0.5f);
      ImVec2 thumbnailMax(thumbnailMin.x + thumbnailSize, thumbnailMin.y + thumbnailSize);
      drawList->AddRectFilled(thumbnailMin, thumbnailMax, IM_COL32(25, 37, 52, textAlpha), 3.0f);
      if (rowLevelData.has_value() && !rowLevelData->image.empty()){
        auto thumbnail = gameapi->getTextureSamplerId(rowLevelData->image);
        if (thumbnail.has_value()){
          drawList->AddImage(
            (ImTextureID)(intptr_t)thumbnail.value(),
            thumbnailMin,
            thumbnailMax,
            ImVec2(0, 1),
            ImVec2(1, 0),
            IM_COL32(255, 255, 255, textAlpha)
          );
        }
      }

      float numberX = thumbnailMax.x + 9.0f;
      float textY = rowMin.y + (rowSize.y - ImGui::GetFontSize()) * 0.5f;
      drawList->AddText(ImVec2(numberX, textY), IM_COL32(91, 204, 207, textAlpha), stageNumber.c_str());
      float titleX = numberX + 30.0f;
      float titleMaxX = isLocked(index) ? rowMin.x + rowSize.x - 68.0f : rowMin.x + rowSize.x - 10.0f;
      if (titleMaxX > titleX){
        drawList->PushClipRect(ImVec2(titleX, rowMin.y), ImVec2(titleMaxX, rowMin.y + rowSize.y), true);
        drawList->AddText(ImVec2(titleX, textY), textColor, rowTitle.c_str());
        drawList->PopClipRect();
      }
      if (isLocked(index)){
        drawList->AddText(ImVec2(rowMin.x + rowSize.x - 60.0f, rowMin.y + (rowSize.y - ImGui::GetFontSize()) * 0.5f), IM_COL32(230, 184, 96, (int)(220 * fade)), "LOCKED");
      }
      ImGui::PopID();
    }
    drawList->AddRectFilledMultiColor(
      ImVec2(rightX + 2.0f, bodyY + 42.0f),
      ImVec2(rightX + rightWidth - 2.0f, bodyY + 78.0f),
      IM_COL32(7, 11, 18, 230),
      IM_COL32(7, 11, 18, 230),
      IM_COL32(7, 11, 18, 0),
      IM_COL32(7, 11, 18, 0)
    );
    drawList->AddRectFilledMultiColor(
      ImVec2(rightX + 2.0f, bodyY + bodyHeight - 41.0f),
      ImVec2(rightX + rightWidth - 2.0f, bodyY + bodyHeight - 5.0f),
      IM_COL32(7, 11, 18, 0),
      IM_COL32(7, 11, 18, 0),
      IM_COL32(7, 11, 18, 230),
      IM_COL32(7, 11, 18, 230)
    );
    drawList->PopClipRect();

    ImGui::SetCursorScreenPos(ImVec2(origin.x, origin.y + available.y));
    ImGui::Dummy(ImVec2(available.x, 1.0f));

    drawListSplitter.Merge(drawList);
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