#include "./util.h"

extern CustomApiBindings* gameapi;
extern ConsoleInterface consoleInterface;

static bool showConsoleLog = false;
extern AiData aiData;
extern GameTypes gametypeSystem;
extern GlobalState global;
extern std::unordered_map<objid, HitPoints> hitpoints;
extern std::unordered_map<objid, Inventory> scopenameToInventory;

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


///////////
extern CustomApiBindings* gameapi;


const float bufferExpirationTimeMs = 5000;
const int maxBufferSize = 1;

enum AlertMessageType { ALERT_DETAIL };
struct AlertMessage {
  std::string message;
  std::optional<double> time;
  AlertMessageType type;
};
struct Alerts {
  std::deque<AlertMessage> messageBuffer;
};

std::string amountToDraw2(std::string& text, double createTime, float rate){
  auto currIndex = static_cast<int>((gameapi -> timeSeconds(true) - createTime) * rate);
  return text.substr(0, currIndex);
}

const int letterSize = 8;
const float letterSizeNdi = letterSize / 1000.f;
const float margin = letterSizeNdi * 3;
const float marginLeft = margin;
const float marginBottom = margin;


void renderAlerts2(Alerts& alerts, int yoffset, std::deque<AlertMessage>& buffer){
  for (int i = 0; i < buffer.size(); i++){
    AlertMessage& message = buffer.at(i);
    if (message.type != ALERT_DETAIL){
      continue;
    }
    if (!message.time.has_value()){
      message.time = gameapi -> timeSeconds(true);
    }

    auto textToDraw = amountToDraw2(message.message, message.time.value(), 100);
    /*gameapi -> drawText(
      textToDraw, 
      (-1 + marginLeft),
      (-1 + (letterSizeNdi * 0.5) + marginBottom), 
      letterSize, 
      false, 
      std::nullopt, 
      std::nullopt,
      true, 
      std::nullopt, 
      std::nullopt,
      std::nullopt,
      std::nullopt
    );*/

    drawImGuiText(textToDraw);

    break;
  }
}

bool isNotExpiredMessage2(AlertMessage& message){
  if (!message.time.has_value()){
    return true;
  }
  auto currTime = gameapi -> timeSeconds(true);
  auto createTime = message.time.value();
  auto diff = (currTime - createTime) * 1000;
  return diff < bufferExpirationTimeMs;
}

void filterExpiredMessages2(Alerts& alerts){
  std::deque<AlertMessage> newMessageBuffer;
  for (auto &message : alerts.messageBuffer){
    if (isNotExpiredMessage2(message)){
      newMessageBuffer.push_back(message);
    }
  }
  alerts.messageBuffer = newMessageBuffer;
}

Alerts alerts {
  .messageBuffer = {},
};

void pushAlertMessage(std::string message){
  std::cout << "push alert message: " << message << std::endl;
   alerts.messageBuffer.push_back(AlertMessage {
    .message = message,
    .time = std::nullopt,
    .type = ALERT_DETAIL,
   });
   if (alerts.messageBuffer.size() > maxBufferSize){
    alerts.messageBuffer.pop_front();
   }
}

void onAlertFrame(){
  renderAlerts2(alerts, 400, alerts.messageBuffer);
  filterExpiredMessages2(alerts); // probably shouldn't be done every frame
}

void renderAnimations(bool includePanel){
  if (includePanel){
    ImGui::Begin("Debug Animations");
  }

  auto playerIndex = getDefaultPlayerIndex();
  bool isControlledPlayer = hasControlledPlayer(playerIndex);
  if (!isControlledPlayer){
      ImGui::Text("No Controlled Player");
  }else{
      ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);
      ImGui::Text("Controlled Player");

      std::vector<objid> ids;
      if (controlledPlayer.entityId.has_value()){
        ids.push_back(controlledPlayer.entityId.value());
      }
      if (ids.size() == 0){
        ImGui::Text("No Entity");
      }else{
        ImGui::Text("Entity Id: ");
        ImGui::SameLine();
        ImGui::Text(std::to_string(controlledPlayer.entityId.value()).c_str());
      }
      if (ids.size() > 0){
        auto id = ids.at(0);
        auto name = gameapi -> getGameObjNameForId(id).value();
        auto animationNames = gameapi -> listAnimations(id);
        ImGui::Text("Num Animations");
        ImGui::SameLine();
        ImGui::Text(std::to_string(animationNames.size()).c_str());
      
        int index = 0;
        for (auto& animation : animationNames){
          bool isNamePose = animation.find("pose-") == 0;
          ImGui::Text("Animation: ");
          ImGui::SameLine();
          ImGui::Text(animation.c_str());
          ImGui::SameLine();

          ImGui::PushID(index);
          index++;
          if (ImGui::Button("Play")){
            if (isNamePose){
              gameapi -> setAnimationPose(id, animation, 0.f);
            }else{
              gameapi -> playAnimation(id, animation, ONESHOT, std::nullopt, 0, false, std::nullopt);
            }
          }
          ImGui::PopID();
        }
      }
  }

  if (includePanel){
    ImGui::End();
  }
}

void renderGameType(bool includePanel){
  if (includePanel){
    ImGui::Begin("Debug GameType");
  }

  ImGui::Text("num agents: ");
  ImGui::SameLine();
  ImGui::Text(std::to_string(aiData.agents.size()).c_str());

  if (!gametypeSystem.meta){
    ImGui::Text("no gametype");
  }else{
    ImGui::Text(gametypeSystem.name.c_str());
  }

  ImGui::Text("Show Editor: ");
  ImGui::SameLine();
  ImGui::Text(global.showEditor ? "true" : "false");

  ImGui::Text("Show Console: ");
  ImGui::SameLine();
  ImGui::Text(global.systemConfig.showConsole ? "true" : "false");

  ImGui::Text("Show Keyboard: ");
  ImGui::SameLine();
  ImGui::Text(global.systemConfig.showKeyboard ? "true" : "false");
  ImGui::SameLine();
  if (ImGui::Button("Toggle Keyboard")){
    global.systemConfig.showKeyboard = !global.systemConfig.showKeyboard;
  }


  ImGui::Text("routeState.paused: ");
  ImGui::SameLine();
  ImGui::Text(global.routeState.paused ? "true" : "false");

  ImGui::Text("routeState.inGameMode: ");
  ImGui::SameLine();
  ImGui::Text(global.routeState.inGameMode ? "true" : "false");

  ImGui::Text("routeState.showMouse: ");
  ImGui::SameLine();
  ImGui::Text(global.routeState.showMouse ? "true" : "false");


  auto playerIndex = getDefaultPlayerIndex();


  {
    std::string activeCameraName = "";
    auto activeCameraId = gameapi -> getActiveCamera(std::nullopt);
    if (activeCameraId.has_value()){
      activeCameraName = gameapi -> getGameObjNameForId(activeCameraId.value()).value();
    }
    ImGui::Text("Active Name: ");
    ImGui::SameLine();
    ImGui::Text(activeCameraName.c_str());
  }

  {
    std::string playerName = "";
    std::string cameraName = "";
    bool isControlledPlayer = hasControlledPlayer(playerIndex);
    if (isControlledPlayer){
      ControlledPlayer& controlledPlayer = getControlledPlayer(playerIndex);
      if (controlledPlayer.entityId.has_value()){
        playerName = gameapi -> getGameObjNameForId(controlledPlayer.entityId.value()).value();
      }      
      if (controlledPlayer.activePlayerManagedCameraId.has_value()){
        cameraName = gameapi -> getGameObjNameForId(controlledPlayer.activePlayerManagedCameraId.value()).value();
      }
    }

    ImGui::Text("Player Obj Name");
    ImGui::SameLine();
    ImGui::Text(playerName == "" ? "[no player name]" : playerName.c_str());

    ImGui::Text("activePlayerManagedCameraId Name");
    ImGui::SameLine();
    ImGui::Text(cameraName == "" ? "[no active camera name]" : cameraName.c_str());
  }

  {
    auto inThirdPerson = entityInThirdPersonByPlayerIndex(playerIndex);
    ImGui::Text("Mode: ");
    ImGui::SameLine();
    if (inThirdPerson.has_value()){
      ImGui::Text(inThirdPerson.value() ? "Third" : "First");
    }else{
      ImGui::Text("Not entity");
    }
  }

  if (includePanel){
    ImGui::End();
  }  
}

void renderHitpoints(bool includePanel){
  if (includePanel){
    ImGui::Begin("Debug Hitpoints");
  }

  ImGui::Text("Num Managed: ");
  ImGui::SameLine();
  ImGui::Text(std::to_string(hitpoints.size()).c_str());

  ImGui::Separator();

  for (auto &[id, hitpoint] : hitpoints){
    ImGui::Text(std::to_string(id).c_str());
    ImGui::SameLine();
    auto name = gameapi -> getGameObjNameForId(id).value();
    ImGui::Text(name.c_str());
    ImGui::SameLine();
    ImGui::Text(std::to_string(hitpoint.current).c_str());
  }


  if (includePanel){
    ImGui::End();
  }
}

void renderInventory(bool includePanel){
  if (includePanel){
    ImGui::Begin("Debug Inventory");
  }

  ImGui::Text("Num Inventories: ");
  ImGui::SameLine();
  ImGui::Text(std::to_string(scopenameToInventory.size()).c_str());

  for (auto& [id, inventory] : scopenameToInventory){
    ImGui::Text(std::to_string(id).c_str());
    ImGui::SameLine();
    ImGui::Text(inventory.infinite ? "infinite" : "");
    ImGui::Indent();
    for (auto& [item, _] : inventory.items){
      ImGui::Text(item.c_str());
    }
    ImGui::Unindent();
  }

  if (includePanel){
    ImGui::End();
  }
}

