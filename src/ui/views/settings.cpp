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

bool success = false;
const char* SETTINGS_SAVE_FILE = "../afterworld/data/save/settings.json";
float getSaveFloatValue(std::string key, float defaultValue){
  auto data = gameapi -> loadFromJsonFile (SETTINGS_SAVE_FILE, &success);
  if (!success){
    data = {};
  }
  if (data.find("settings") == data.end()){
    data["settings"] = {};
  }

  if (data.at("settings").find(key) == data.at("settings").end()){
    return defaultValue;
  }

  JsonType value = data.at("settings").at(key);
  float* floatValue = std::get_if<float>(&value);
  if (floatValue){
    return *floatValue;
  }
  return defaultValue;
}
float getSaveBoolValue(std::string key, float defaultValue){
  auto data = gameapi -> loadFromJsonFile (SETTINGS_SAVE_FILE, &success);
  if (!success){
    data = {};
  }
  if (data.find("settings") == data.end()){
    data["settings"] = {};
  }

  if (data.at("settings").find(key) == data.at("settings").end()){
    return defaultValue;
  }
  
  JsonType value = data.at("settings").at(key);
  bool* boolValue = std::get_if<bool>(&value);
  if (boolValue){
    return *boolValue;
  }
  return defaultValue;
}

std::string getSaveStringValue(std::string key, std::string defaultValue){
  auto data = gameapi -> loadFromJsonFile (SETTINGS_SAVE_FILE, &success);
  if (!success){
    data = {};
  }
  if (data.find("settings") == data.end()){
    data["settings"] = {};
  }

  if (data.at("settings").find(key) == data.at("settings").end()){
    return defaultValue;
  }
  
  JsonType value = data.at("settings").at(key);
  std::string* strValue = std::get_if<std::string>(&value);
  if (strValue){
    return *strValue;
  }
  return defaultValue;
}

void persistSave(std::string key, JsonType value){
  auto data = gameapi -> loadFromJsonFile (SETTINGS_SAVE_FILE, &success);
  if (!success){
    data = {};
  }
  if (data.find("settings") == data.end()){
    data["settings"] = {};
  }
  data.at("settings")[key] = value;
  gameapi -> saveToJsonFile(SETTINGS_SAVE_FILE, data);
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

float originalFov = 45;
void setZoom(float percentage, bool hideGun){
  gameapi -> setLayerState({
      StrValues {
        .target = "",
        .attribute = "fov",
        .payload = std::to_string(originalFov * percentage),
      },
  });    
  gameapi -> setLayerState({
      StrValues {
        .target = "transparency",
        .attribute = "fov",
        .payload = std::to_string(originalFov * percentage),
      },
  });    

  gameapi -> setLayerState({
      StrValues {
        .target = "no_depth",
        .attribute = "visible",
        .payload = hideGun ? "false" : "true",
      },
  });  
}

std::vector<std::pair<std::string, std::vector<SettingConfiguration>>> settingsItems {
  { "Game", std::vector<SettingConfiguration> {
    SettingConfiguration {
      .config = DockCheckboxConfig {
        .label = "Invert Aim",
        .isChecked = []() -> bool { return getGlobalState().invertY; },
        .onChecked = [](bool isChecked) -> void {
          getGlobalState().invertY = isChecked;
          persistSave("invertY", isChecked);
        },
      },
      .initSetting = []() -> void {
        getGlobalState().invertY = getSaveBoolValue("invertY", false);
      },
    },
    SettingConfiguration {
      .config = DockSliderConfig {
        .label = "X-Sensitivity",
        .min = 0.f,
        .max = 5.f,
        .percentage = []() -> float { return getGlobalState().xsensitivity; },
        .onSlide = [](float amount) -> void {
          getGlobalState().xsensitivity = amount;
          persistSave("xsensitivity", amount);
        },
      },
      .initSetting = []() -> void {
        getGlobalState().xsensitivity = getSaveFloatValue("xsensitivity", 1.f);
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
          persistSave("ysensitivity", amount);
        },
      },
      .initSetting = []() -> void {
        getGlobalState().ysensitivity = getSaveFloatValue("ysensitivity", 1.f);
      },
    },
  }},
  { "Graphics", std::vector<SettingConfiguration> {
    SettingConfiguration {
      .config = DockCheckboxConfig {
        .label = "Fullscreen",
        .isChecked = getIsCheckedWorld("rendering", "fullscreen"),
        .onChecked = [](bool isChecked) -> void {
          bool isFullscreen = isChecked;
          persistSave("fullscreen", isFullscreen);
          dockConfigApi.setAttribute("rendering", "fullscreen", isFullscreen);
        },
      },
      .initSetting = []() -> void {
        bool isFullscreen = getSaveBoolValue("fullscreen", true);
        dockConfigApi.setAttribute("rendering", "fullscreen", isFullscreen);
      },
    },
    SettingConfiguration {
      .config = DockSliderConfig {
        .label = "FOV",
        .min = 20.f,
        .max = 50.f,
        .percentage = []() -> float { 
          return originalFov;
        },
        .onSlide = [](float amount) -> void {
          originalFov = amount;
          persistSave("fov", originalFov);
          setZoom(1.f, false);
        },
      },
      .initSetting = []() -> void {
        auto fov = getSaveFloatValue("fov", 45.f);
        originalFov = fov;
        setZoom(1.f, false);
      },
    },
  }},
  { "Controls", std::vector<SettingConfiguration> {
    SettingConfiguration {
      .config = DockTextboxNumeric {
        .label = "Jump",
        .value = []() -> std::string { return "1.0"; },
        .onEdit = [](float, std::string&) -> void { },
      },
      .initSetting = std::nullopt,
    },
    SettingConfiguration {
      .config = DockTextboxNumeric {
        .label = "Up",
        .value = []() -> std::string { return "1.0"; },
        .onEdit = [](float, std::string&) -> void { },
      },
      .initSetting = std::nullopt,
    },
    SettingConfiguration {
      .config = DockTextboxNumeric {
        .label = "Down",
        .value = []() -> std::string { return "1.0"; },
        .onEdit = [](float, std::string&) -> void { },
      },
      .initSetting = std::nullopt,
    },
    SettingConfiguration {
      .config = DockTextboxNumeric {
        .label = "Left",
        .value = []() -> std::string { return "1.0"; },
        .onEdit = [](float, std::string&) -> void { },
      },
      .initSetting = std::nullopt,
    },
    SettingConfiguration {
      .config = DockTextboxNumeric {
        .label = "Right",
        .value = []() -> std::string { return "1.0"; },
        .onEdit = [](float, std::string&) -> void { },
      },
      .initSetting = std::nullopt,
    },
    SettingConfiguration {
      .config = DockTextboxNumeric {
        .label = "Shoot",
        .value = []() -> std::string { return "1.0"; },
        .onEdit = [](float, std::string&) -> void { },
      },
      .initSetting = std::nullopt,
    },
    SettingConfiguration {
      .config = DockTextboxNumeric {
        .label = "Aim",
        .value = []() -> std::string { return "1.0"; },
        .onEdit = [](float, std::string&) -> void { },
      },
      .initSetting = std::nullopt,
    },
    SettingConfiguration {
      .config = DockTextboxNumeric {
        .label = "Crouch",
        .value = []() -> std::string { return "1.0"; },
        .onEdit = [](float, std::string&) -> void { },
      },
      .initSetting = std::nullopt,
    },
    SettingConfiguration {
      .config = DockTextboxNumeric {
        .label = "Pickup",
        .value = []() -> std::string { return "1.0"; },
        .onEdit = [](float, std::string&) -> void { },
      },
      .initSetting = std::nullopt,
    },
  }},
  { "Sound", std::vector<SettingConfiguration> {
    SettingConfiguration {
      .config = DockCheckboxConfig {
        .label = "Sound Enabled",
        .isChecked = getIsCheckedWorldInvert("sound", "mute"),
        .onChecked = [](bool isChecked) -> void {
          bool mute = !isChecked;
          persistSave("mute", mute);
          dockConfigApi.setAttribute("sound", "mute", mute);
        },
      },
      .initSetting = []() -> void {
        bool mute = getSaveBoolValue("mute", false);
        dockConfigApi.setAttribute("sound", "mute", mute);
      },
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
          persistSave("volume", amount);
        },
      },
      .initSetting = []() -> void {
        auto volume = getSaveFloatValue("volume", 1.f);
        dockConfigApi.setAttribute("sound", "volume", volume);
      },
    },

    SettingConfiguration {
      .config = DockSliderConfig {
        .label = "Gameplay Volume",
        .min = 0.f,
        .max = 1.f,
        .percentage = []() -> float { 
          return getGameplayVolume();
        },
        .onSlide = [](float amount) -> void {
          setGameplayVolume(amount);
        },
      },
      .initSetting = std::nullopt,
    },
    SettingConfiguration {
      .config = DockSliderConfig {
        .label = "Music Volume",
        .min = 0.f,
        .max = 1.f,
        .percentage = []() -> float { 
          return getMusicVolume();
        },
        .onSlide = [](float amount) -> void {
          setMusicVolume(amount);
        },
      },
      .initSetting = std::nullopt,
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