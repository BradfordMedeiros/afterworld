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



bool keyIsDown(int key);
bool leftMouseDown();
bool middleMouseDown();
bool rightMouseDown();
glm::vec2 getMouseVelocity();

struct KeyLocation {
  int key;
  glm::vec2 topLeft;
  glm::vec2 bottomRight;
};

std::vector<KeyLocation> keys {
  KeyLocation {
    .key = 'Q',
    .topLeft = glm::vec2(-0.750332f, 0.348781f),
    .bottomRight = glm::vec2(-0.648074f, 0.0926829f),
  },
  KeyLocation {
    .key = 'W',
    .topLeft = glm::vec2(-0.615228f, 0.343885f),
    .bottomRight = glm::vec2(-0.513256f, 0.0964029f),
  },
  KeyLocation {
    .key = GLFW_KEY_ESCAPE,
    .topLeft = glm::vec2(-0.953912, 0.926489),
    .bottomRight = glm::vec2(-0.842444, 0.764259),
  },
  KeyLocation {
    .key = GLFW_KEY_F1,
    .topLeft = glm::vec2(-0.815648, 0.921420),
    .bottomRight = glm::vec2(-0.700965, 0.764259),
  },
  KeyLocation {
    .key = GLFW_KEY_F2,
    .topLeft = glm::vec2(-0.677385, 0.923954),
    .bottomRight = glm::vec2(-0.561629, 0.764259),
  },
  KeyLocation {
    .key = GLFW_KEY_F3,
    .topLeft = glm::vec2(-0.539121, 0.916350),
    .bottomRight = glm::vec2(-0.425509, 0.764259),
  },
  KeyLocation {
    .key = GLFW_KEY_F4,
    .topLeft = glm::vec2(-0.399786, 0.926489),
    .bottomRight = glm::vec2(-0.286174, 0.751584),
  },
  KeyLocation {
    .key = GLFW_KEY_F5,
    .topLeft = glm::vec2(-0.257235, 0.923954),
    .bottomRight = glm::vec2(-0.141479, 0.759189),
  },
  KeyLocation {
    .key = GLFW_KEY_F6,
    .topLeft = glm::vec2(-0.113612, 0.918885),
    .bottomRight = glm::vec2(-0.005359, 0.759189),
  },
  KeyLocation {
    .key = GLFW_KEY_F7,
    .topLeft = glm::vec2(0.031083, 0.898606),
    .bottomRight = glm::vec2(0.132905, 0.751584),
  },
  KeyLocation {
    .key = GLFW_KEY_F8,
    .topLeft = glm::vec2(0.178993, 0.903676),
    .bottomRight = glm::vec2(0.273312, 0.756654),
  },
  KeyLocation {
    .key = GLFW_KEY_F9,
    .topLeft = glm::vec2(0.323687, 0.908745),
    .bottomRight = glm::vec2(0.410504, 0.759189),
  },
  KeyLocation {
    .key = GLFW_KEY_F10,
    .topLeft = glm::vec2(0.440514, 0.906210),
    .bottomRight = glm::vec2(0.547696, 0.759189),
  },
  KeyLocation {
    .key = GLFW_KEY_F11,
    .topLeft = glm::vec2(0.572347, 0.923954),
    .bottomRight = glm::vec2(0.685959, 0.761724),
  },
  KeyLocation {
    .key = GLFW_KEY_F12,
    .topLeft = glm::vec2(0.712755, 0.929024),
    .bottomRight = glm::vec2(0.827438, 0.759189),
  },
  KeyLocation {
    .key = 32, // spacebar
    .topLeft = glm::vec2(-0.381457f, -0.626635f),
    .bottomRight = glm::vec2(0.255629f, -0.921522f),
  },
  KeyLocation {
    .key = 'A',
    .topLeft = glm::vec2(-0.720373, 0.017921),
    .bottomRight = glm::vec2(-0.612517, -0.230585),
  },
  KeyLocation {
    .key = 'S',
    .topLeft = glm::vec2(-0.583222, 0.003584),
    .bottomRight = glm::vec2(-0.475366, -0.235364),
  },
  KeyLocation {
    .key = 'D',
    .topLeft = glm::vec2(-0.444740, 0.017921),
    .bottomRight = glm::vec2(-0.339547, -0.230585),
  },
  KeyLocation {
    .key = 'F',
    .topLeft = glm::vec2(-0.315579, 0.020311),
    .bottomRight = glm::vec2(-0.211718, -0.230585),
  },
  KeyLocation {
    .key = 'G',
    .topLeft = glm::vec2(-0.183755, 0.025090),
    .bottomRight = glm::vec2(-0.073236, -0.235364),
  },
  KeyLocation {
    .key = 'H',
    .topLeft = glm::vec2(-0.049268, 0.025090),
    .bottomRight = glm::vec2(0.058589, -0.237754),
  },
  KeyLocation {
    .key = 'J',
    .topLeft = glm::vec2(0.081225, 0.034648),
    .bottomRight = glm::vec2(0.194407, -0.242533),
  },
  KeyLocation {
    .key = 'J',
    .topLeft = glm::vec2(0.083888, 0.032258),
    .bottomRight = glm::vec2(0.195739, -0.230585),
  },
  KeyLocation {
    .key = 'K',
    .topLeft = glm::vec2(0.217044, 0.027479),
    .bottomRight = glm::vec2(0.326232, -0.242533),
  },
  KeyLocation {
    .key = 'L',
    .topLeft = glm::vec2(0.356858, 0.027479),
    .bottomRight = glm::vec2(0.464714, -0.230585),
  },
  KeyLocation {
    .key = ';',
    .topLeft = glm::vec2(0.484687, 0.022700),
    .bottomRight = glm::vec2(0.591212, -0.240143),
  },
  KeyLocation {
    .key = '\'',
    .topLeft = glm::vec2(0.619174, 0.032258),
    .bottomRight = glm::vec2(0.727031, -0.240143),
  },
  KeyLocation {
    .key = 'Z',
    .topLeft = glm::vec2(-0.654877, -0.300380),
    .bottomRight = glm::vec2(-0.542337, -0.561470),
  },
  KeyLocation {
    .key = 'X',
    .topLeft = glm::vec2(-0.521972, -0.292776),
    .bottomRight = glm::vec2(-0.405145, -0.566540),
  },
  KeyLocation {
    .key = 'C',
    .topLeft = glm::vec2(-0.384780, -0.292776),
    .bottomRight = glm::vec2(-0.273312, -0.564005),
  },
  KeyLocation {
    .key = 'V',
    .topLeft = glm::vec2(-0.249732, -0.295310),
    .bottomRight = glm::vec2(-0.140407, -0.564005),
  },
  KeyLocation {
    .key = 'B',
    .topLeft = glm::vec2(-0.116827, -0.300380),
    .bottomRight = glm::vec2(-0.008574, -0.564005),
  },
  KeyLocation {
    .key = 'N',
    .topLeft = glm::vec2(0.016077, -0.297845),
    .bottomRight = glm::vec2(0.128617, -0.566540),
  },
  KeyLocation {
    .key = 'M',
    .topLeft = glm::vec2(0.151125, -0.292776),
    .bottomRight = glm::vec2(0.261522, -0.564005),
  },
  KeyLocation {
    .key = ',',
    .topLeft = glm::vec2(0.287402, -0.298734),
    .bottomRight = glm::vec2(0.395013, -0.554430),
  },
  KeyLocation {
    .key = '.',
    .topLeft = glm::vec2(0.418635, -0.296203),
    .bottomRight = glm::vec2(0.531496, -0.556962),
  },
  KeyLocation {
    .key = '/',
    .topLeft = glm::vec2(0.551181, -0.286076),
    .bottomRight = glm::vec2(0.665354, -0.559494),
  },


  KeyLocation {
    .key = 'E',
    .topLeft = glm::vec2(-0.484444, 0.356052),
    .bottomRight = glm::vec2(-0.373968, 0.083431),
  },
  KeyLocation {
    .key = 'R',
    .topLeft = glm::vec2(-0.351111, 0.349001),
    .bottomRight = glm::vec2(-0.240635, 0.088132),
  },
  KeyLocation {
    .key = 'T',
    .topLeft = glm::vec2(-0.216508, 0.356052),
    .bottomRight = glm::vec2(-0.106032, 0.088132),
  },
  KeyLocation {
    .key = 'Y',
    .topLeft = glm::vec2(-0.081905, 0.349001),
    .bottomRight = glm::vec2(0.026032, 0.090482),
  },
  KeyLocation {
    .key = 'O',
    .topLeft = glm::vec2(0.318095, 0.353702),
    .bottomRight = glm::vec2(0.431111, 0.088132),
  },
  KeyLocation {
    .key = 'P',
    .topLeft = glm::vec2(0.452364f, 0.347291f),
    .bottomRight = glm::vec2(0.561044f, 0.0960591f),
  },
  KeyLocation {
    .key = 'U',
    .topLeft = glm::vec2(0.052519, 0.372340),
    .bottomRight = glm::vec2(0.160772, 0.111702),
  },
  KeyLocation {
    .key = 'I',
    .topLeft = glm::vec2(0.185077, 0.373810),
    .bottomRight = glm::vec2(0.297732, 0.104762),
  },
  KeyLocation {
    .key = '[',
    .topLeft = glm::vec2(0.584906, 0.355603),
    .bottomRight = glm::vec2(0.696934, 0.092672),
  },
  KeyLocation {
    .key = ']',
    .topLeft = glm::vec2(0.720519, 0.355603),
    .bottomRight = glm::vec2(0.826651, 0.090517),
  },
  KeyLocation {
    .key = '\\',
    .topLeft = glm::vec2(0.854953, 0.355603),
    .bottomRight = glm::vec2(0.966981, 0.088362),
  },
  KeyLocation {
    .key = '`',
    .topLeft = glm::vec2(-0.957128, 0.691489),
    .bottomRight = glm::vec2(-0.844587, 0.428191),
  },
  KeyLocation {
    .key = '1',
    .topLeft = glm::vec2(-0.822079, 0.678074),
    .bottomRight = glm::vec2(-0.713826, 0.404309),
  },
  KeyLocation {
    .key = '2',
    .topLeft = glm::vec2(-0.687031, 0.678074),
    .bottomRight = glm::vec2(-0.574491, 0.404309),
  },
  KeyLocation {
    .key = '3',
    .topLeft = glm::vec2(-0.549839, 0.678074),
    .bottomRight = glm::vec2(-0.440514, 0.404309),
  },
  KeyLocation {
    .key = '4',
    .topLeft = glm::vec2(-0.414791, 0.678074),
    .bottomRight = glm::vec2(-0.306538, 0.404309),
  },
  KeyLocation {
    .key = '5',
    .topLeft = glm::vec2(-0.281886, 0.691489),
    .bottomRight = glm::vec2(-0.171490, 0.438830),
  },
  KeyLocation {
    .key = '6',
    .topLeft = glm::vec2(-0.144695, 0.696809),
    .bottomRight = glm::vec2(-0.040729, 0.444149),
  },
  KeyLocation {
    .key = '7',
    .topLeft = glm::vec2(-0.016077, 0.678074),
    .bottomRight = glm::vec2(0.098607, 0.404309),
  },
  KeyLocation {
    .key = '8',
    .topLeft = glm::vec2(0.118971, 0.678074),
    .bottomRight = glm::vec2(0.227224, 0.404309),
  },
  KeyLocation {
    .key = '9',
    .topLeft = glm::vec2(0.251876, 0.678074),
    .bottomRight = glm::vec2(0.363344, 0.404309),
  },
  KeyLocation {
    .key = '0',
    .topLeft = glm::vec2(0.386924, 0.696809),
    .bottomRight = glm::vec2(0.493033, 0.441489),
  },
  KeyLocation {
    .key = '-',
    .topLeft = glm::vec2(0.518757, 0.696809),
    .bottomRight = glm::vec2(0.628081, 0.433511),
  },
  KeyLocation {
    .key = '=',
    .topLeft = glm::vec2(0.652733, 0.691489),
    .bottomRight = glm::vec2(0.765273, 0.430851),
  },


////

  KeyLocation {
  .key = GLFW_KEY_TAB,
  .topLeft = glm::vec2(-0.957128, 0.373891),
  .bottomRight = glm::vec2(-0.778135, 0.102662),
  },
  KeyLocation {
    .key = GLFW_KEY_CAPS_LOCK,
    .topLeft = glm::vec2(-0.954984, 0.044360),
    .bottomRight = glm::vec2(-0.747053, -0.214195),
  },
  KeyLocation {
    .key = GLFW_KEY_LEFT_SHIFT,
    .topLeft = glm::vec2(-0.957128, -0.287706),
    .bottomRight = glm::vec2(-0.679528, -0.541191),
  },
  KeyLocation {
    .key = GLFW_KEY_LEFT_CONTROL,
    .topLeft = glm::vec2(-0.958199, -0.604563),
    .bottomRight = glm::vec2(-0.845659, -0.918885),
  },
  KeyLocation {
    .key = GLFW_KEY_LEFT_ALT,
    .topLeft = glm::vec2(-0.688103, -0.604563),
    .bottomRight = glm::vec2(-0.572347, -0.918885),
  },
  KeyLocation {
    .key = GLFW_KEY_RIGHT_ALT,
    .topLeft = glm::vec2(0.451233, -0.595745),
    .bottomRight = glm::vec2(0.565916, -0.912234),
  },
  KeyLocation {
    .key = GLFW_KEY_LEFT,
    .topLeft = glm::vec2(0.587353, -0.766793),
    .bottomRight = glm::vec2(0.697749, -0.901141),
  },
  KeyLocation {
    .key = GLFW_KEY_RIGHT,
    .topLeft = glm::vec2(0.854234, -0.754119),
    .bottomRight = glm::vec2(0.964630, -0.908745),
  },
  KeyLocation {
    .key = GLFW_KEY_UP,
    .topLeft = glm::vec2(0.719185, -0.596958),
    .bottomRight = glm::vec2(0.832797, -0.738910),
  },
  KeyLocation {
    .key = GLFW_KEY_DOWN,
    .topLeft = glm::vec2(0.722401, -0.754119),
    .bottomRight = glm::vec2(0.830654, -0.916350),
  },

  KeyLocation {
    .key = GLFW_KEY_ENTER,
    .topLeft = glm::vec2(0.752412, 0.053191),
    .bottomRight = glm::vec2(0.961415, -0.215425),
  },
  KeyLocation {
    .key = GLFW_KEY_RIGHT_SHIFT,
    .topLeft = glm::vec2(0.684887, -0.263298),
    .bottomRight = glm::vec2(0.964630, -0.545213),
  },
  KeyLocation {
    .key = GLFW_KEY_BACKSPACE,
    .topLeft = glm::vec2(0.784566, 0.696809),
    .bottomRight = glm::vec2(0.963558, 0.420213),
  },
};

void drawKeyHighlighted(KeyLocation& key, glm::vec2 size){
  glm::vec2 topLeft = key.topLeft;
  glm::vec2 bottomRight = key.bottomRight;
  topLeft.x *= size.x;
  bottomRight.x *= size.x;
  topLeft.y *= size.y;
  bottomRight.y *= size.y;

  float height = topLeft.y - bottomRight.y;
  float width = bottomRight.x - topLeft.x;
  float midpointX = (topLeft.x + bottomRight.x) / 2.f;
  float midpointY = (bottomRight.y + topLeft.y) / 2.f;


  gameapi->drawRect(
    midpointX,
    midpointY,
    width,
    height,
    false,
    glm::vec4(0.f, 0.f, 1.f, 0.6f),
    std::nullopt,
    true,
    std::nullopt,
    std::nullopt,
    std::nullopt
  );
}


void drawKeyboardVisualization(){
  glm::vec2 size(0.5f, 0.5f);
  gameapi->drawRect(
    0.f,
    0.f,
    size.x,
    size.y,
    false,
    glm::vec4(1.f, 1.f, 1.f, 0.8f),
    std::nullopt,
    true,
    std::nullopt,
    paths::UI_KEYBOARD_IMAGE,
    std::nullopt
  );
  for (auto &key : keys){
    if (keyIsDown(key.key)){
      drawKeyHighlighted(key, size * 0.5f); // 0.5f since this is using ndi
    }
  }
}

void drawMouseButtonValue(glm::vec2 size, float xoffset, bool selected){
  gameapi->drawRect(xoffset, 0.f, size.x, size.y, false, selected ? glm::vec4(0.f, 0.f, 1.f, 0.8f) : glm::vec4(1.f, 1.f, 1.f, 0.6f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
}

void drawMouseVisualization(){
  glm::vec2 size(0.05f, 0.05f);
      
  float totalWidth = size.x * 3;

  drawMouseButtonValue(size, size.x * 0 + (size.x * 0.5f) - (totalWidth * 0.5f), leftMouseDown());
  drawMouseButtonValue(size, size.x * 1 + (size.x * 0.5f) - (totalWidth * 0.5f), middleMouseDown());
  drawMouseButtonValue(size, size.x * 2 + (size.x * 0.5f) - (totalWidth * 0.5f), rightMouseDown());

  auto mouseVel = getMouseVelocity();

  gameapi->drawText(std::to_string(static_cast<int>(mouseVel.x)), -size.x, 0.f, 0.02f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
  gameapi->drawText(std::to_string(static_cast<int>(mouseVel.y)), size.x, 0.f, 0.02f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);

}

void drawInputVisualization(){
  drawKeyboardVisualization();
  drawMouseVisualization();
}



// fade 

extern CustomApiBindings* gameapi;

LetterboxFade letterbox {
  .title = "",
  .animationDuration = 0.f,
  .animationHold = 0.f,
  .fadeOutDuration = 0.f,
  .boxColor = glm::vec4(0.f, 0.f, 0.f, 0.8f),
  .fadeColor = glm::vec4(0.1f, 0.1f, 0.1f, 0.0f),
  .fontSize = 0.02f,
};



std::optional<float> calculateFade(LetterboxFade& fade, std::optional<float> letterBoxStartTime){
  if (!letterBoxStartTime.has_value()){
    return std::nullopt;
  }
  float elapsedTime = gameapi -> timeSeconds(false) - letterBoxStartTime.value();
  float holdStart = (letterbox.animationDuration.has_value() ? letterbox.animationDuration.value() : 0.f);
  float fadeOutStart = holdStart + (letterbox.animationHold.has_value() ? letterbox.animationHold.value() : 0.f);
  float fadeEnd = fadeOutStart + (letterbox.fadeOutDuration.has_value() ? letterbox.fadeOutDuration.value() : 0.f);

  if (elapsedTime < holdStart){
    if (!letterbox.animationDuration.has_value()){
      return 1.f;
    }
    float percentage = elapsedTime / letterbox.animationDuration.value();
    return percentage;
  }else if (elapsedTime >= holdStart && elapsedTime < fadeOutStart){
    return 1.f;
  }else if (elapsedTime < fadeEnd) {
    if (!letterbox.fadeOutDuration.has_value()){
      return 0.f;
    }
    float percentage = (elapsedTime - fadeOutStart) / letterbox.animationDuration.value();
    return (1.f - percentage);
  }

  return std::nullopt;
}

std::optional<float> letterBoxStartTime = std::nullopt;

void showLetterBoxDetail(std::string title, float fadeOutDuration, std::optional<glm::vec4> backgroundColor, glm::vec4 boxColor, float holdDuration, float fadeIn){
  letterbox = LetterboxFade {
    .title = title,
    .animationDuration = fadeIn,
    .animationHold = holdDuration,
    .fadeOutDuration = fadeOutDuration,
    .boxColor = boxColor,
    .fadeColor = backgroundColor,
    .fontSize = 0.02f,
  };
  letterBoxStartTime = gameapi -> timeSeconds(false);
}

void showLetterBox(std::string title, float duration){
  showLetterBoxDetail(title, duration * 0.25f, glm::vec4(0.f, 0.f, 0.f, 0.2f), glm::vec4(0.f, 0.f, 0.f, 0.8f), duration * 0.5f, duration * 0.25f);
}
void showLetterBoxHold(std::string title, float fadeInTime){
  showLetterBoxDetail(title, 0.f, std::nullopt, glm::vec4(1.f, 1.f, 1.f, 0.8f), 10000000.f, fadeInTime);
}
void hideLetterBox(){
  letterBoxStartTime = std::nullopt;
}

void drawFade(){
  auto fade = calculateFade(letterbox, letterBoxStartTime);
  if (fade.has_value()){
    float percentage = fade.value();
    {
      // background
      if (letterbox.fadeColor.has_value()){
        gameapi->drawRect(0.f, 0.f, 2.f, 2.f, false, glm::vec4(letterbox.fadeColor.value().x, letterbox.fadeColor.value().y, letterbox.fadeColor.value().z, letterbox.fadeColor.value().w * percentage), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
      }
    }
    { // title borders
      modlog("ui border", std::string("percentage is: ") + std::to_string(percentage));
      float barHeight = 0.2f * percentage;
      gameapi->drawRect(0.f, 1.f - (barHeight * 0.5f), 2.f, barHeight, false, letterbox.boxColor, std::nullopt, true, std::nullopt, "./res/textures/wood.jpg", std::nullopt);
      gameapi->drawRect(0.f, -1.f + (barHeight * 0.5f), 2.f, barHeight, false, letterbox.boxColor, std::nullopt, true, std::nullopt, "./res/textures/wood.jpg", std::nullopt);
      const float textPaddingRight = 0.04f;
      float textWidth;
      float textHeight;

      float ndiSize = 0.1f;
      float fontSizeNdiEquivalent = ndiSize * 1000.f / 2.f;   // 1000 = 1 ndi

      gameapi -> getTextDimensionsNdi(letterbox.title, fontSizeNdiEquivalent, true, std::nullopt, &textWidth, &textHeight);
      gameapi -> drawText(letterbox.title, 0.f, 0.f, fontSizeNdiEquivalent, false, std::nullopt, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
    }
  }else {
    letterBoxStartTime = std::nullopt;
  }
}
