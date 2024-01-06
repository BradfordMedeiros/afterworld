#include "./settings.h"

extern DockConfigApi dockConfigApi;
extern CustomApiBindings* gameapi;

std::function<void()> getExecuteSqlBool(std::string field, std::string enabled, std::string disabled, std::string worldStateObj, std::string worldStateAttr, std::string valueEnabled, std::string valueDisabled){
  return [field, enabled, disabled, worldStateObj, worldStateAttr, valueEnabled, valueDisabled]() -> void {
    auto query = gameapi -> compileSqlQuery("select ? from settings", { field });
    bool validSql = false;
    auto result = gameapi -> executeSqlQuery(query, &validSql);
    modassert(validSql, "error executing sql query");
    std::string value = result.at(0).at(0);
    modassert(value == enabled || value == disabled, "invalid value executeSqlPersist");
    auto isEnabled = value == enabled;
    dockConfigApi.setAttribute(worldStateObj, worldStateAttr, isEnabled ? valueEnabled : valueDisabled);
  };
}

std::function<void(bool)> persistSql(std::string column, std::string enabled, std::string disabled, std::function<void(bool)> onCheckedFn){
  return [column, enabled, disabled, onCheckedFn](bool value) -> void {
    onCheckedFn(value);
    auto strValue = value ? enabled : disabled;
    auto updateQuery = gameapi -> compileSqlQuery("update settings set ? = ?", { column, strValue });
    bool validSql = false;
    gameapi -> executeSqlQuery(updateQuery, &validSql);
    modassert(validSql, "error executing sql query");

  };
};

std::string getSqlValue(std::string column){
  auto query = gameapi -> compileSqlQuery("select ? from settings", { column });
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(query, &validSql);
  modassert(validSql, "error executing sql query");
  return result.at(0).at(0).c_str();
}
float getSqlFloatValue(std::string column){
  auto query = gameapi -> compileSqlQuery("select ? from settings", { column });
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(query, &validSql);
  modassert(validSql, "error executing sql query");
  float value = std::atof(result.at(0).at(0).c_str());
  return value;
}

float getWorldStateFloat(std::string object, std::string attribute){
  auto valueAttr = dockConfigApi.getAttribute(object, attribute);
  float* floatPtr = std::get_if<float>(&valueAttr);
  modassert(floatPtr, object + " is not a float");
  return *floatPtr;
}


struct SettingConfiguration {
  DockConfig config;
  std::optional<std::function<void()>> initSetting;
};

float currentFov = 45;
std::vector<std::pair<std::string, std::vector<SettingConfiguration>>> settingsItems {
  { "Game", std::vector<SettingConfiguration> {
    SettingConfiguration {
      .config = DockCheckboxConfig {
        .label = "Invert Aim",
        .isChecked = []() -> bool { return getGlobalState().invertY; },
        .onChecked = [](bool isChecked) -> void {
          getGlobalState().invertY = isChecked;
        },
      },
      .initSetting = std::nullopt,
    },
    SettingConfiguration {
      .config = DockSliderConfig {
        .label = "X-Sensitivity",
        .min = 0.f,
        .max = 5.f,
        .percentage = []() -> float { return getGlobalState().xsensitivity; },
        .onSlide = [](float amount) -> void {
          getGlobalState().xsensitivity = amount;
          persistSqlFloat("xsensitivity", amount);
        },
      },
      .initSetting = []() -> void {
        getGlobalState().xsensitivity = getSqlFloatValue("xsensitivity");
      },
    },
    SettingConfiguration {
      .config = DockSliderConfig {
        .label = "Y-Sensitivity",
        .min = 0.f,
        .max = 5.f,
        .percentage = []() -> float { return getGlobalState().ysensitivity; },
        .onSlide = [](float amount) -> void {
          getGlobalState().ysensitivity = amount;
          persistSqlFloat("ysensitivity", amount);
        },
      },
      .initSetting = []() -> void {
        getGlobalState().ysensitivity = getSqlFloatValue("ysensitivity");
      },
    },
  }},
  { "Graphics", std::vector<SettingConfiguration> {
    SettingConfiguration {
      .config = DockCheckboxConfig {
        .label = "Fullscreen",
        .isChecked = getIsCheckedWorld("rendering", "fullscreen", "true", "false"),
        .onChecked = persistSql(
          "fullscreen", "TRUE", "FALSE",
          getOnCheckedWorld("rendering", "fullscreen", "true", "false")
        ),
      },
      .initSetting = getExecuteSqlBool("fullscreen", "TRUE", "FALSE", "rendering", "fullscreen", "true", "false"),
    },
    SettingConfiguration {
      .config = DockSliderConfig {
        .label = "FOV",
        .min = 20.f,
        .max = 50.f,
        .percentage = []() -> float { 
          return currentFov;
        },
        .onSlide = [](float amount) -> void {
          currentFov = amount;
          persistSqlFloat("fov", currentFov);
          gameapi -> setLayerState({
            StrValues {
              .target = "",
              .attribute = "fov",
              .payload = std::to_string(currentFov),
            },
          });         
        },
      },
      .initSetting = []() -> void {
        auto fov = getSqlValue("fov");
        currentFov = std::atof(fov.c_str());
        gameapi -> setLayerState({
          StrValues {
            .target = "",
            .attribute = "fov",
            .payload = fov,
          },
        });
      },
    },
  }},
  { "Controls", std::vector<SettingConfiguration> {
    SettingConfiguration {
      .config = DockTextboxNumeric {
        .label = "Placeholder",
        .value = 1.f,
      },
      .initSetting = std::nullopt,
    },
  }},
  { "Sound", std::vector<SettingConfiguration> {
    SettingConfiguration {
      .config = DockCheckboxConfig {
        .label = "Sound Enabled",
        .isChecked = getIsCheckedWorld("sound", "mute", "false", "true"),
        .onChecked = persistSql(
          "mute", "FALSE", "TRUE",
          getOnCheckedWorld("sound", "mute", "false", "true")
        ),
      },
      .initSetting = getExecuteSqlBool("mute", "TRUE", "FALSE", "sound", "mute", "true", "false"),
    },
    SettingConfiguration {
      .config = DockSliderConfig {
        .label = "Volume",
        .min = 0.f,
        .max = 1.f,
        .percentage = []() -> float { 
          auto volume = getWorldStateFloat("sound", "volume");
          return volume;
        },
        .onSlide = [](float amount) -> void {
          // sound:volume:0.2
          dockConfigApi.setAttribute("sound", "volume", amount);
          persistSqlFloat("volume", amount);
        },
      },
      .initSetting = []() -> void {
        auto volume = getSqlFloatValue("volume");
        dockConfigApi.setAttribute("sound", "volume", volume);
      },
    },
  }},
};



void addConfiguration(std::vector<DockConfiguration>& allConfigurations, std::vector<SettingConfiguration>& configuration, std::string title){
  DockConfiguration dockConfig {
    .title = title,
    .configFields = {},
  };
  for (SettingConfiguration& config : configuration){
    dockConfig.configFields.push_back(config.config);
  }
  allConfigurations.push_back(dockConfig);
}

std::vector<DockConfiguration> loadConfigurations(){
  std::vector<DockConfiguration> settingsConfigurations {};
  for (auto &settingsItem : settingsItems){
    addConfiguration(settingsConfigurations, settingsItem.second, settingsItem.first);
  }
  return settingsConfigurations;
}

std::vector<DockConfiguration> settingsConfigurations = loadConfigurations();

void initSettings(){
  for (auto &settingsItem : settingsItems){
    std::vector<SettingConfiguration>& settingsConfig = settingsItem.second;
    for (auto &config : settingsConfig){
      if (config.initSetting.has_value()){
        config.initSetting.value()();
      }
    }
  }
}


int selectedMenuIndex = -1;
Component settingsInner {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    int currentIndex = 0;
    if (selectedMenuIndex >= 0){
      currentIndex = selectedMenuIndex;
    }

    DockConfiguration* dockConfiguration = &settingsConfigurations.at(currentIndex);
    Props dockProps {
      .props = {
        PropPair { .symbol = valueSymbol, .value = dockConfiguration },
      }
    };
    return dockFormComponent.draw(drawTools, dockProps);
  }
};


Component menuList {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    std::vector<Component> elements;
    for (int i = 0; i < settingsConfigurations.size(); i++){
      std::string& menuItem = settingsConfigurations.at(i).title;
      std::function<void()> onClick = [i]() -> void {
        selectedMenuIndex = i;
      };
      Props listItemProps {
        .props = {
          PropPair { .symbol = valueSymbol, .value = menuItem },
          PropPair { .symbol = paddingSymbol, .value = styles.dockElementPadding },
          PropPair { .symbol = minwidthSymbol, .value = 0.5f },
          PropPair { .symbol = borderColorSymbol, .value = glm::vec4(1.f, 1.f, 1.f, 0.1f) },
          PropPair { .symbol = onclickSymbol, .value = onClick },
        },
      };
      if (selectedMenuIndex == i){
        listItemProps.props.push_back(PropPair {
          .symbol = tintSymbol,
          .value = glm::vec4(1.f, 1.f, 0.f, 0.2f),
        });
      }
      auto listItemWithProps = withPropsCopy(listItem, listItemProps);
      elements.push_back(listItemWithProps);
    }
    Layout outerLayout {
      .tint = styles.primaryColor,
      .showBackpanel = true,
      .borderColor = styles.highlightColor,
      .minwidth = 0.5f,
      .minheight = 2.f,
      .layoutType = LAYOUT_VERTICAL2, 
      .layoutFlowHorizontal = UILayoutFlowNone2,
      .layoutFlowVertical = UILayoutFlowNone2,
      .alignHorizontal = UILayoutFlowNone2,
      .alignVertical = UILayoutFlowPositive2,
      .spacing = 0.f,
      .minspacing = 0.f,
      .padding = 0.f,
      .children = elements,
    };
    Props listLayoutProps {
      .props = {
        { .symbol = layoutSymbol, .value = outerLayout },
      },
    };
    return layoutComponent.draw(drawTools, listLayoutProps);
  }
};

Component settingsComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    static bool firstTime = true;
    if (firstTime){
      initSettings();
      firstTime = false;
    }
    std::vector<Component> elements;
    elements.push_back(menuList);
    elements.push_back(settingsInner);

    Layout outerLayout {
      .tint = glm::vec4(0.f, 0.f, 0.f, 0.f),//styles.secondaryColor,
      .showBackpanel = true,
      .borderColor = styles.highlightColor,
      .minwidth = 2.f,
      .minheight = 2.f,
      .layoutType = LAYOUT_HORIZONTAL2, 
      .layoutFlowHorizontal = UILayoutFlowNone2,
      .layoutFlowVertical = UILayoutFlowNone2,
      .alignHorizontal = UILayoutFlowNegative2,
      .alignVertical = UILayoutFlowNone2,
      .spacing = 0.f,
      .minspacing = 0.f,
      .padding = 0.f,
      .children = elements,
    };
    Props listLayoutProps {
      .props = {
        { .symbol = layoutSymbol, .value = outerLayout },
      },
    };
    auto boundingBox = withProps(layoutComponent, listLayoutProps).draw(drawTools, props);
    return boundingBox;
  }
};