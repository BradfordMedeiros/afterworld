#include "./settings.h"

struct SettingsSqlPersist {
};

struct SettingConfiguration {
  DockConfig config;
  std::optional<SettingsSqlPersist> sqlPersist;
};

std::vector<std::pair<std::string, std::vector<SettingConfiguration>>> settingsItems {
  { "Game", std::vector<SettingConfiguration> {
    SettingConfiguration {
      .config = DockCheckboxConfig {
        .label = "Invert Aim",
        .isChecked = []() -> bool { return false; },
        .onChecked = [](bool) -> void { },
      },
      .sqlPersist = std::nullopt,
    },
    SettingConfiguration {
      .config = DockTextboxNumeric {
        .label = "X-Sensitivity",
        .value = 0.5f,
      },
      .sqlPersist = std::nullopt,
    },
    SettingConfiguration {
      .config = DockTextboxNumeric {
        .label = "Y-Sensitivity",
        .value = 0.5f,
      },
      .sqlPersist = std::nullopt,
    },
  }},
  { "Graphics", std::vector<SettingConfiguration> {
    SettingConfiguration {
      .config = DockCheckboxConfig {
        .label = "Fullscreen",
        .isChecked = getIsCheckedWorld("rendering", "fullscreen", "true", "false"),
        .onChecked = getOnCheckedWorld("rendering", "fullscreen", "true", "false"),
      },
      .sqlPersist = std::nullopt,
    },
    SettingConfiguration {
      .config = DockTextboxNumeric {
        .label = "FOV",
        .value = 10.f,
      },
      .sqlPersist = std::nullopt,
    },
  }},
  { "Controls", std::vector<SettingConfiguration> {
    SettingConfiguration {
      .config = DockTextboxNumeric {
        .label = "Placeholder",
        .value = 1.f,
      },
      .sqlPersist = std::nullopt,
    },
  }},
  { "Sound", std::vector<SettingConfiguration> {
    SettingConfiguration {
      .config = DockCheckboxConfig {
        .label = "Sound Enabled",
        .isChecked = getIsCheckedWorld("sound", "mute", "false", "true"),
        .onChecked = getOnCheckedWorld("sound", "mute", "false", "true"),
      },
      .sqlPersist = std::nullopt,
    },
    SettingConfiguration {
      .config = DockTextboxNumeric {
        .label = "Volume",
        .value = 1.f,
      },
      .sqlPersist = std::nullopt,
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