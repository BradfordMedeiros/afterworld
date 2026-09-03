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

