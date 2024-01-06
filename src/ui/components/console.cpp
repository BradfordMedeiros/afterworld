#include "./console.h"

const int CONSOLE_WIDTH = 2.f;
const int CONSOLE_HEIGHT = 2.f;
const float ELEMENT_PADDING = 0.02f;
const float ELEMENT_WIDTH = 1.f;
const int CONSOLE_LOG_LIMIT = 10;

extern CustomApiBindings* gameapi;

struct HistoryInstance {
  std::string command;
  bool valid;
};

std::deque<HistoryInstance> loadCommandHistory(){
  std::cout << "load command history" << std::endl;
  auto query = gameapi -> compileSqlQuery("select text, valid from history", {});
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(query, &validSql);
  modassert(validSql, "error executing sql query");

  std::deque<HistoryInstance> history;

  int fromIndex = result.size() - CONSOLE_LOG_LIMIT;
  if (fromIndex < 0){
    fromIndex = 0;
  }
  for (int i = fromIndex; i < result.size(); i++){
    auto row = result.at(i);
    history.push_back(
      HistoryInstance {
        .command = row.at(0),      
        .valid = row.at(1) == "true",
      }
    );
  }
  return history;
}

std::vector<char> preventedCharacters = { ',', '#', '\n', '?' };
void insertCommandHistory(std::string& command, bool valid){
  auto query = gameapi -> compileSqlQuery("insert into history (text, valid) values ( ?, ?)", { command, valid ? "true" : "false" });
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(query, &validSql);
  modassert(validSql, "error executing sql query");
}


std::deque<HistoryInstance> commandHistory =  {};
std::deque<HistoryInstance> logHistory = {};

int selectedIndex = 0;
bool showLog = false;

struct CommandDispatch {
  std::string command;
  std::function<std::optional<std::string>(ConsoleInterface&, std::string&, bool*)> fn;
};

std::vector<CommandDispatch> commands {
  CommandDispatch {
    .command = "quit",
    .fn = [](ConsoleInterface& consoleInterface, std::string& commandStr, bool* valid) -> std::optional<std::string> {
      if (commandStr != "quit"){
        *valid = false;
        return std::nullopt;
      }
      exit(1);
      *valid = true;
      return std::nullopt;
    }, 
  },
  CommandDispatch {
    .command = "log",
    .fn = [](ConsoleInterface& consoleInterface, std::string& commandStr, bool* valid) -> std::optional<std::string> {
      showLog = true;
      *valid = true;
      return std::nullopt;
    }, 
  },
  CommandDispatch {
    .command = "history",
    .fn = [](ConsoleInterface& consoleInterface, std::string& commandStr, bool* valid) -> std::optional<std::string> {
      showLog = false;
      *valid = true;
      return std::nullopt;
    }, 
  },
  CommandDispatch {
    .command = "editor",
    .fn = [](ConsoleInterface& consoleInterface, std::string& commandStr, bool* valid) -> std::optional<std::string> {
      auto values = split(commandStr, ' ');
      *valid = false;
      if (values.at(1) == "on"){
        consoleInterface.setShowEditor(true);
        *valid = true;
        return std::nullopt;
      }else if (values.at(1) == "off"){ 
        consoleInterface.setShowEditor(false);
        *valid = true;
        return std::nullopt;
      }
      return std::nullopt;
    }, 
  },
  CommandDispatch {
    .command = "background",
    .fn = [](ConsoleInterface& consoleInterface, std::string& command, bool* valid) -> std::optional<std::string> {
      auto values = split(command, ' ');
      *valid = false;
      if (values.size() != 2){
        return std::nullopt;
      }
      auto backgroundValue = values.at(1);
      consoleInterface.setBackground(backgroundValue);
      *valid = true;
      return std::nullopt;
    },
  },
  CommandDispatch {
    .command = "level",
    .fn = [](ConsoleInterface& consoleInterface, std::string& command, bool* valid) -> std::optional<std::string> {
      *valid = false;
      auto values = split(command, ' ');
      if (values.size() == 1){
        consoleInterface.goToLevel(std::nullopt);
        *valid = true;
        return std::nullopt;
      }
      if (values.size() == 2){
        auto levelName = values.at(1);
        consoleInterface.goToLevel(levelName);
        *valid = true;
        return std::nullopt;
      }
      return std::nullopt;
    },
  },
  CommandDispatch {
    .command = "help",
    .fn = [](ConsoleInterface& consoleInterface, std::string& command, bool* valid) -> std::optional<std::string> {
      *valid = true;
      std::string result = "[commands]\n\n";
      for (auto &command : commands){
        result += command.command + "  ";
      }
      return result;    
    },
  },
  CommandDispatch {
    .command = "router",
    .fn = [](ConsoleInterface& consoleInterface, std::string& command, bool* valid) -> std::optional<std::string> {
      *valid = false;
      auto values = split(command, ' ');
      if (values.size() == 1){
        return std::nullopt;
      }
      if (values.at(1) == "pop"){
        consoleInterface.routerPop();
        return std::nullopt;
      }else if (values.at(1) == "push" && values.size() == 3){
        consoleInterface.routerPush(values.at(2), false);
        return std::nullopt;
      }else if (values.at(1) == "replace"){
        consoleInterface.routerPush(values.at(2), true);
      }

      return std::nullopt;    
    },
  },
};

std::optional<CommandDispatch*> findCommand(std::string commandStr){
  for (auto &command : commands){
    auto splitCommand = split(commandStr, ' ');
    auto mainCommand = splitCommand.at(0);
    if (mainCommand == command.command){
      return &command;
    }
  }
  return std::nullopt;
}


void executeCommand(ConsoleInterface& consoleInterface, std::string command){
  if (commandHistory.size() >= CONSOLE_LOG_LIMIT){
    commandHistory.pop_front();
  }
  selectedIndex--;
  if (selectedIndex < 0){
    selectedIndex = -1;
  }

  std::transform(command.begin(), command.end(), command.begin(), [](unsigned char c) {
    return std::tolower(c);
  });
  auto commandDispatch = findCommand(command);
  if (commandDispatch.has_value()){
    bool valid = false;
    auto result = commandDispatch.value() -> fn(consoleInterface, command, &valid);
    commandHistory.push_back(HistoryInstance {
      .command = command,
      .valid = valid,
    });
    insertCommandHistory(command, valid);

    if (result.has_value()){
      commandHistory.push_back(HistoryInstance {
        .command = result.value(),
        .valid = valid,
      });
    }

  }else{
    commandHistory.push_back(HistoryInstance {
      .command = command,
      .valid = false,
    });
    insertCommandHistory(command, false);
  }
}

Component createTitle(std::string&& item){
  Props listItemProps {
    .props = {
      PropPair { .symbol = valueSymbol, .value = item },
      PropPair { .symbol = tintSymbol, .value = glm::vec4(0.f, 0.f, 0.f, 1.f) },
      PropPair { .symbol = colorSymbol, .value = glm::vec4(1.f, 1.f, 1.f, 0.8f) },
      PropPair { .symbol = minwidthSymbol, .value = ELEMENT_WIDTH },
      PropPair { .symbol = paddingSymbol, .value = ELEMENT_PADDING },
      PropPair { .symbol = fontsizeSymbol, .value = 0.03f },
      //PropPair { .symbol = onclickSymbol, .value = onClick },
    },
  };
  auto listItemWithProps = withPropsCopy(listItem, listItemProps); 
  return listItemWithProps;
}

Component createConsoleItem(HistoryInstance& history,  int index){
  std::function<void(int)> onClick2 = [index, &history](int value) -> void {
    if (value == 0 /* left click*/){
      selectedIndex = index;
    }else if (value == 2 /* middle mouse click*/){
      gameapi -> setClipboardString(history.command.c_str());
      selectedIndex = index;
    }
  };

  Props listItemProps {
    .props = {
      PropPair { .symbol = valueSymbol, .value = history.command },
      PropPair { .symbol = colorSymbol, .value = glm::vec4(1.f, 1.f, 0.f, 0.8f) },
      PropPair { .symbol = minwidthSymbol, .value = ELEMENT_WIDTH },
      PropPair { .symbol = paddingSymbol, .value = ELEMENT_PADDING },
      PropPair { .symbol = onclickRightSymbol, .value = onClick2 },
      PropPair { .symbol = borderColorSymbol, .value = glm::vec4(1.f, 1.f, 1.f, 0.2f) },
    },
  };
  if (selectedIndex == index){
    listItemProps.props.push_back(PropPair { .symbol = tintSymbol, .value = glm::vec4(0.4f, 0.4f, 0.4f, 0.8f) });
  }else if (!history.valid){
    listItemProps.props.push_back(PropPair { .symbol = tintSymbol, .value = glm::vec4(1.f, 0.f, 0.f, 0.2f) });
  }
  auto listItemWithProps = withPropsCopy(listItem, listItemProps); 
  return listItemWithProps;
}

TextData commandTextData {
  .valueText = "",
  .cursorLocation = 0,
  .highlightLength = 0,
  .maxchars = -1,
};
Component consoleComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    static bool firstTime = true;
    if (firstTime){
      commandHistory = loadCommandHistory();
      gameapi -> setLogEndpoint([](std::string& message) -> void {
        if (logHistory.size() >= CONSOLE_LOG_LIMIT){
          logHistory.pop_front();
        }
        logHistory.push_back(HistoryInstance {
          .command = message,
          .valid = true,
        });
      });
    }
    firstTime = false;
  	std::vector<Component> elements = {};

    elements.push_back(createTitle(showLog ? "Log" : "History"));

    std::deque<HistoryInstance>* consoleSource = showLog ? &logHistory : &commandHistory;

    for (int i = 0; i < consoleSource -> size(); i++){
      elements.push_back(createConsoleItem(consoleSource -> at(i), i));
    }
    int paddingElements = CONSOLE_LOG_LIMIT - consoleSource -> size();
    if (paddingElements > 0){
      for (int i = 0; i < paddingElements; i++){
        std::cout << "paddingElements: " << paddingElements << std::endl;
        HistoryInstance instance {
          .command = "",
          .valid = true,
        };
        elements.push_back(createConsoleItem(instance, i));
      }
    }

    ConsoleInterface** consoleInterfacePtr = typeFromProps<ConsoleInterface*>(props, consoleInterfaceSymbol);
    modassert(consoleInterfacePtr, "console interface not provided");
    ConsoleInterface* consoleInterface = *consoleInterfacePtr;
    std::function<void(TextData, int)> onEdit = [consoleInterface](TextData textData, int rawKey) -> void {
      if (rawKey == 257 /* enter */ ){
        if (commandTextData.valueText.size() > 0){
          std::string commandStr = commandTextData.valueText;
          commandTextData.valueText = "";
          commandTextData.cursorLocation = 0;
          commandTextData.highlightLength = 0;
          executeCommand(*consoleInterface, commandStr);
        }
      }else{
        commandTextData = textData;
      }
    };


    auto consoleKey = typeFromProps<std::string>(props, autofocusSymbol);
    Props textboxProps {
      .props = {
        PropPair { .symbol = editableSymbol, .value = true },
        PropPair { .symbol = textDataSymbol, .value = commandTextData },
        PropPair { .symbol = onInputSymbol, .value = onEdit },
        PropPair { .symbol = minwidthSymbol, .value = ELEMENT_WIDTH },
        PropPair { .symbol= tintSymbol, .value = glm::vec4(0.f, 0.f, 0.f, 1.f) },
        PropPair { .symbol= colorSymbol, .value = glm::vec4(1.f, 1.f, 1.f, 1.f) },
        PropPair { .symbol= borderColorSymbol, .value = glm::vec4(1.f, 1.f, 1.f, 1.f) },
      }
    };
    if (consoleKey){
      textboxProps.props.push_back(PropPair { .symbol = autofocusSymbol, .value = *consoleKey });
    }

    auto textboxWithProps = withPropsCopy(textbox, textboxProps);
    elements.push_back(textboxWithProps);

  	Layout layout {
  	  .tint = glm::vec4(0.f, 0.f, 0.f, 0.9f),
  	  .showBackpanel = true,
  	  .borderColor = glm::vec4(1.f, 1.f, 1.f, 1.f),
  	  .minwidth = CONSOLE_WIDTH,
  	  .minheight = CONSOLE_HEIGHT,
  	  .layoutType = LAYOUT_VERTICAL2,
  	  .layoutFlowHorizontal = UILayoutFlowNone2,
  	  .layoutFlowVertical = UILayoutFlowNone2,
  	  .alignHorizontal = UILayoutFlowNegative2,
  	  .alignVertical = UILayoutFlowPositive2,
  	  .spacing = 0.f,
  	  .minspacing = 0.f,
  	  .padding = styles.dockElementPadding,
  	  .children = elements,
  	};
  	Props listLayoutProps {
  	  .props = {
  	    { .symbol = layoutSymbol, .value = layout },
  	  },
  	};
  	auto layoutScenegraph = withPropsCopy(layoutComponent, listLayoutProps);
  	auto boundingBox = layoutScenegraph.draw(drawTools, props);
    return boundingBox;
  },
};