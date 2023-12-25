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
  std::function<bool(ConsoleInterface&, std::string&)> fn;
};

std::vector<CommandDispatch> commands {
  CommandDispatch {
    .command = "quit",
    .fn = [](ConsoleInterface& consoleInterface, std::string& commandStr) -> bool {
      if (commandStr != "quit"){
        return false;
      }
      exit(1);
      return true;
    }, 
  },
  CommandDispatch {
    .command = "log",
    .fn = [](ConsoleInterface& consoleInterface, std::string& commandStr) -> bool {
      showLog = true;
      return true;
    }, 
  },
  CommandDispatch {
    .command = "history",
    .fn = [](ConsoleInterface& consoleInterface, std::string& commandStr) -> bool {
      showLog = false;
      return true;
    }, 
  },
  CommandDispatch {
    .command = "editor",
    .fn = [](ConsoleInterface& consoleInterface, std::string& commandStr) -> bool {
      auto values = split(commandStr, ' ');
      if (values.at(1) == "on"){
        consoleInterface.setShowEditor(true);
        return true;
      }else if (values.at(1) == "off"){ 
        consoleInterface.setShowEditor(false);
        return true;
      }
      return false;
    }, 
  },
  CommandDispatch {
    .command = "background",
    .fn = [](ConsoleInterface& consoleInterface, std::string& command) -> bool {
      auto values = split(command, ' ');
      if (values.size() != 2){
        return false;
      }
      auto backgroundValue = values.at(1);
m      consoleInterface.setBackground(backgroundValue);
      return true;
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
    bool valid = commandDispatch.value() -> fn(consoleInterface, command);
    commandHistory.push_back(HistoryInstance {
      .command = command,
      .valid = valid,
    });
    insertCommandHistory(command, valid);
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
  std::function<void()> onClick = [index]() -> void {
    std::cout << "on click" << std::endl;
    selectedIndex = index;
  };
  Props listItemProps {
    .props = {
      PropPair { .symbol = valueSymbol, .value = history.command },
      PropPair { .symbol = colorSymbol, .value = glm::vec4(1.f, 1.f, 0.f, 0.8f) },
      PropPair { .symbol = minwidthSymbol, .value = ELEMENT_WIDTH },
      PropPair { .symbol = paddingSymbol, .value = ELEMENT_PADDING },
      PropPair { .symbol = onclickSymbol, .value = onClick },
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
    std::function<void(TextData)> onEdit = [consoleInterface](TextData textData) -> void {
      if (textData.valueText.size() > 0 && static_cast<int>(textData.valueText.at(textData.valueText.size() - 1)) == 10 /* enter */ ){
        auto commandStr = textData.valueText.substr(0, textData.valueText.size() -1);
        commandTextData.valueText = "";
        commandTextData.cursorLocation = 0;
        commandTextData.highlightLength = 0;
        executeCommand(*consoleInterface, commandStr);
      }else{
        commandTextData = textData;
      }
    };

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