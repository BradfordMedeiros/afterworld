#include "./dock.h"

extern DockConfigApi dockConfigApi;

const float STYLE_UI_DOCK_ELEMENT_PADDING = 0.02f;

void persistSqlFloat(std::string column, float value){
  auto updateQuery = gameapi -> compileSqlQuery("update settings set ? = ?", { column, std::to_string(value) });
  bool validSql = false;
  gameapi -> executeSqlQuery(updateQuery, &validSql); 
}

struct SqlFilter {
  std::string column;
  std::string value;
};
void persistSql(std::string table, std::string column, std::string value, std::optional<SqlFilter> filter = std::nullopt){
  if (filter.has_value()){
    auto updateQuery = gameapi -> compileSqlQuery("update ? set ? = ? where ? = ?", { table, column, value, filter.value().column, filter.value().value });
    bool validSql = false;
    gameapi -> executeSqlQuery(updateQuery, &validSql); 
    return;
  }
  auto updateQuery = gameapi -> compileSqlQuery("update ? set ? = ?", { table, column, value });
  bool validSql = false;
  gameapi -> executeSqlQuery(updateQuery, &validSql); 
}

std::string readSqlFirstRow(std::string table, std::string column, std::optional<SqlFilter> filter = std::nullopt){
  if (filter.has_value()){
    auto updateQuery = gameapi -> compileSqlQuery("select ? from ? where ? = ?", { column, table, filter.value().column, filter.value().value });
    bool validSql = false;
    auto result = gameapi -> executeSqlQuery(updateQuery, &validSql); 
    return result.at(0).at(0);
  }
  auto updateQuery = gameapi -> compileSqlQuery("select ? from ?", { column, table });
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(updateQuery, &validSql); 
  return result.at(0).at(0);
}

bool weaponsExpanded = false;
int weaponSelectIndex = -1;
std::optional<std::string> selectedGun;
std::vector<std::string> listGuns(){
  auto updateQuery = gameapi -> compileSqlQuery("select name from guns", {});
  bool validSql = false;
  auto result = gameapi -> executeSqlQuery(updateQuery, &validSql);
  std::vector<std::string> guns;
  for (auto &gunRow : result){
    guns.push_back(gunRow.at(0));
  }
  return guns;
}

DockCheckboxConfig createSimpleCheckbox(const char* table, const char* label, const char* columnName, std::function<std::optional<SqlFilter>()> getFilter){
  DockCheckboxConfig checkbox {
    .label = label,
    .isChecked = [table, columnName, getFilter]() -> bool {
      auto filter = getFilter();
      if (!filter.has_value()){
        return false;
      }
      auto sqlValue = readSqlFirstRow(table, columnName, filter.value());
      return sqlValue == "TRUE";
    },
    .onChecked = [table, columnName, getFilter](bool checked) -> void {
      auto filter = getFilter();
      if (!filter.has_value()){
        return;
      }
      persistSql(table, columnName, checked ? "TRUE" : "FALSE", filter.value());
    },
  };
  return checkbox;
}
DockCheckboxConfig createSimpleGunCheckbox(const char* label, const char* columnName){
  return createSimpleCheckbox("guns", label, columnName, []() -> std::optional<SqlFilter> {
    if (!selectedGun.has_value()){
      return std::nullopt;
    }
    return SqlFilter { .column = "name", .value = selectedGun.value() }; 
  });
}

std::string gravityAmount = "10";
DockTextboxNumeric createSimpleTextboxNumeric(const char* table, const char* label, const char* columnName, std::function<std::optional<SqlFilter>()> getFilter = []() -> std::optional<SqlFilter> { return std::nullopt; }){
  DockTextboxNumeric textbox {
    .label = label,
    .value = []() -> std::string { return gravityAmount; },
    .onEdit = [table, columnName, getFilter](float newvalue, std::string& newStr) -> void {
      gravityAmount = newStr;
      if (!getFilter().has_value()){
        return;
      }
      persistSql(table, columnName, serializeFloat(newvalue), getFilter().value());
    },
    // gameobj:water-viscosity  // positive number
  };
  return textbox;
}

std::unordered_map<std::string, bool> collapseValues;
std::function<void()> createCollapsableOnClick(const char* value){
  collapseValues[value] = true;
  return [value]() -> void { collapseValues[value] = !collapseValues.at(value); };
}
std::function<bool()> createShouldBeCollapse(const char* value){
  return [value]() -> bool { return collapseValues.at(value); };
}

std::unordered_map<std::string, std::string> textStore;
enum TextEditType { 
  TEXT_TYPE_STRING, 
  TEXT_TYPE_NUMBER, TEXT_TYPE_POSITIVE_NUMBER, TEXT_TYPE_INTEGER, TEXT_TYPE_POSITIVE_INTEGER,
  TEXT_TYPE_VEC2, TEXT_TYPE_VEC3, TEXT_TYPE_VEC4
};

std::function<std::string()> connectGetText(std::string key){
  if (textStore.find(key) == textStore.end()){
    textStore[key] = "";
  }
  return [key]() -> std::string {
    return textStore.at(key);
  };
}

std::function<std::string()> connectGetTextVec2(std::string key){
  if (textStore.find(key) == textStore.end()){
    textStore[key] = "";
  }
  return [key]() -> std::string {
    auto attr = dockConfigApi.getObjAttr(key);
    if (!attr.has_value()){
      return "";
    }
    auto value = std::get_if<std::string>(&attr.value());
    if (!value){
      return "";
    }
    return *value;
  };
}

std::function<void(std::string)> connectEditTextVec2(std::string key, const char* objKey){
  return [key, objKey](std::string value) -> void {
    if (value.size() == 0){
      textStore[key] = value;
    }
    auto vec2Value = toVec2(value);
    std::cout << "setting vec2Value: " << (vec2Value.has_value() ? "true" : "false") << std::endl;
    if (vec2Value.has_value()){
      textStore[key] = value;

      std::string asString = std::to_string(vec2Value.value().x) + " " + std::to_string(vec2Value.value().y);
      dockConfigApi.setObjAttr(objKey, asString);
    }
  };
}


std::function<void(std::string)> connectEditText(std::string key, TextEditType type = TEXT_TYPE_STRING){
  return [key, type](std::string value) -> void {
    if (value.size() == 0){
      textStore[key] = value;
      return;
    }
    if (type == TEXT_TYPE_STRING){
      textStore[key] = value;
    }else if (type == TEXT_TYPE_NUMBER && toNumber(value).has_value()){
      textStore[key] = value;
    }else if (type == TEXT_TYPE_POSITIVE_NUMBER && toPositiveNumber(value).has_value()){
      textStore[key] = value;
    }else if (type == TEXT_TYPE_INTEGER && toInteger(value).has_value()){
      textStore[key] = value;
    }else if (type == TEXT_TYPE_POSITIVE_INTEGER && toPositiveInteger(value).has_value()){
      textStore[key] = value;
    }else if (type == TEXT_TYPE_VEC2 && toVec2(value).has_value()){
      textStore[key] = value;
    }else if (type == TEXT_TYPE_VEC3 && toVec3(value).has_value()){
      textStore[key] = value;
    }else if (type == TEXT_TYPE_VEC4 && toVec4(value).has_value()){
      textStore[key] = value;
    }
  };
}

std::function<std::string()> floatParticleGetValue(const char* field){
  return [field]() -> std::string {
    auto floatAttribute = dockConfigApi.getParticleAttribute(field);
    if (!floatAttribute.has_value()){
      return "0.0";
    }
    auto floatAttr = std::get_if<float>(&floatAttribute.value());
    modassert(floatAttr, "invalid value floatParticleGetValue");
    return std::to_string(*floatAttr); 
  };
}

std::function<void(float, std::string&)> floatParticleSetValue(const char* field){
  return [field](float value, std::string&) -> void { 
    dockConfigApi.setParticleAttribute(field, value);
  };
}

std::function<std::string()> floatParticleGetValueVec3(const char* field, int index){
  return [field, index]() -> std::string {
    auto vec3Attribute = dockConfigApi.getParticleAttribute(field);
    if (!vec3Attribute.has_value()){
      return "0.0";
    }
    auto vecAttr = std::get_if<glm::vec3>(&vec3Attribute.value());
    modassert(vecAttr, "invalid value vecAttr");
    return std::to_string((*vecAttr)[index]); 
  };
}

std::function<void(float, std::string&)> floatParticleSetValueVec3(const char* field, int index){
  return [field, index](float value, std::string&) -> void { 
    auto vec3Attribute = dockConfigApi.getParticleAttribute(field);
    modassert(vec3Attribute.has_value(), "vec3Attribute is null");
    auto vecAttr = std::get_if<glm::vec3>(&vec3Attribute.value());
    modassert(vecAttr, "vecAttr is null");
    glm::vec3 vecValue = *vecAttr;
    vecValue[index] = value;
    dockConfigApi.setParticleAttribute(field, vecValue);
  };
}

std::function<bool()> floatParticleGetValueBool(const char* field, const char* enabled, const char* disabled){
  return [field, enabled, disabled]() -> bool {
    auto attr = dockConfigApi.getParticleAttribute(field);
    if (attr.has_value()){
      auto strValue = std::get_if<std::string>(&attr.value());
      if (strValue){
        if (*strValue == enabled){
          return true;
        }else if (*strValue == disabled){
          return false;
        }else{
          modassert(false, std::string("invalid value: ") + std::string(field));
        }
      }
      return false;
    }
    return false;
  };
}

std::function<void(bool)> floatParticleSetValueBool(const char* field, const char* enabled, const char* disabled){
  return [field, enabled, disabled](bool isChecked) -> void {
    dockConfigApi.setParticleAttribute(field, isChecked ? enabled : disabled);
  };
}

std::vector<const char*> enemyTypes { "crawler", "tv", "enemy", "turret" };

std::vector<DockConfiguration> configurations {
  DockConfiguration {
    .title = "",
    .configFields = {
      DockButtonConfig {
        .buttonText = "no panel available",
        .onClick = []() -> void {},
      },
    },
  },

  // Gameplay Docks //////////////////////////////////////////////
  //////////////////////////////////////////////////////////////
  DockConfiguration {
    .title = "TRIGGERS",
    .configFields = {
      DockButtonConfig {
        .buttonText = "Create Trigger",
        .onClick = []() -> void {
          std::unordered_map<std::string, AttributeValue> attrs;
          std::string triggerFile("../afterworld/scenes/prefabs/gameplay/trigger.rawscene");
          dockConfigApi.createPrefab(triggerFile, attrs);
        },
      },
      DockCheckboxConfig {
        .label = "Oneshot",
        .isChecked = []() -> bool {
          auto value = dockConfigApi.getObjAttr("+trigger|switch-remove");
          if (!value.has_value()){
            return false;
          }
          auto strValue = std::get_if<std::string>(&value.value());
          if (strValue == NULL){
            return false;
          }
          return *strValue == "|enter";
        },
        .onChecked = [](bool checked) -> void {
          if (checked){
            dockConfigApi.setObjAttr("+trigger|switch-remove", "|enter");
          }else{
            dockConfigApi.setObjAttr("+trigger|switch-remove", DeleteAttribute{});
          }
        },
      },
      DockCheckboxConfig {
        .label = "On Enter",
        .isChecked = []() -> bool {
          auto value = dockConfigApi.getObjAttr("+trigger|switch-enter");
          return value.has_value();
        },
        .onChecked = [](bool checked) -> void {
          if (checked){
            dockConfigApi.setObjAttr("+trigger|switch-enter", "|enter");
          }else{
            dockConfigApi.setObjAttr("+trigger|switch-enter", DeleteAttribute{});
          }   
        },
      },
      DockTextboxConfig {
        .label = "On Enter Key",
        .text = []() -> std::string {
          auto value = dockConfigApi.getObjAttr("+trigger|switch-enter");
          if (!value.has_value()){
            return "[disabled]";
          }
          auto attrValue = value.value();
          auto strValue = std::get_if<std::string>(&attrValue);
          modassert(strValue, "invalid type onEnterKey");
          return strValue -> substr(1, strValue -> size());
        },
        .onEdit = [](std::string value) -> void {
          auto enterValue = dockConfigApi.getObjAttr("+trigger|switch-enter");
          if (!enterValue.has_value()){
            return;
          }
          dockConfigApi.setObjAttr("+trigger|switch-enter", std::string("|") + value);
        }
      },
      DockCheckboxConfig {
        .label = "On Exit",
        .isChecked = []() -> bool {
          auto value = dockConfigApi.getObjAttr("+trigger|switch-exit");
          return value.has_value();
        },
        .onChecked = [](bool checked) -> void {
          if (checked){
            dockConfigApi.setObjAttr("+trigger|switch-exit", "|exit");
          }else{
            dockConfigApi.setObjAttr("+trigger|switch-exit", DeleteAttribute{});
          }   
        },
      },
      DockTextboxConfig {
        .label = "On Exit Key",
        .text = []() -> std::string {
          auto value = dockConfigApi.getObjAttr("+trigger|switch-exit");
          if (!value.has_value()){
            return "[disabled]";
          }
          auto attrValue = value.value();
          auto strValue = std::get_if<std::string>(&attrValue);
          modassert(strValue, "invalid type onEnterKey");
          return strValue -> substr(1, strValue -> size());
        },
        .onEdit = [](std::string value) -> void {
          auto enterValue = dockConfigApi.getObjAttr("+trigger|switch-exit");
          if (!enterValue.has_value()){
            return;
          }
          dockConfigApi.setObjAttr("+trigger|switch-exit", std::string("|") + value);
        }
      },
    }
  },

  //// Editor Docks ////////////////
  /////////////////////////////////////


};

DockConfiguration* dockConfigByName(std::string name){
  for (DockConfiguration& config : configurations){
    if (config.title == name){
      return &config;
    }
  }
  return NULL;
}

void componentsForFields(std::vector<DockConfig>& configFields, std::vector<Component>& elements);
Component createDockComponent(DockConfig& config){
  auto dockLabel = std::get_if<DockLabelConfig>(&config);
  if (dockLabel){
    return createDockLabel(*dockLabel);
  }

  auto dockButton = std::get_if<DockButtonConfig>(&config);
  if (dockButton){
    return createDockButton(*dockButton);
  }

  auto dockOptions = std::get_if<DockOptionConfig>(&config);
  if (dockOptions){
    return createDockOptions(*dockOptions);
  }

  auto sliderOptions = std::get_if<DockSliderConfig>(&config);
  if (sliderOptions){
    return createDockSlider(*sliderOptions);
  }

  auto checkboxOptions = std::get_if<DockCheckboxConfig>(&config);
  if (checkboxOptions){
    return createDockCheckbox(*checkboxOptions);
  }

  auto textboxOptions = std::get_if<DockTextboxConfig>(&config);
  if (textboxOptions){
    return createDockTextbox(*textboxOptions);
  }

  auto imageConfigOptions = std::get_if<DockImageConfig>(&config);
  if (imageConfigOptions){
    return createDockImage(*imageConfigOptions);
  }

  auto gameobjSelectorOptions = std::get_if<DockGameObjSelector>(&config);
  if (gameobjSelectorOptions){
    return createDockGameobj(*gameobjSelectorOptions);
  }

  auto dockGroupOptions = std::get_if<DockGroup>(&config);
  if (dockGroupOptions){
    std::vector<Component> elements;
    auto titleTextbox = withPropsCopy(listItem, Props {
      .props = {
        PropPair { .symbol = valueSymbol, .value = dockGroupOptions -> groupName },
        PropPair { .symbol = onclickSymbol, .value = dockGroupOptions -> onClick },
        PropPair { .symbol = fontsizeSymbol, .value = 0.02f },
        PropPair { .symbol = paddingSymbol, .value = 0.015f },
        PropPair { .symbol = tintSymbol, .value = styles.thirdColor },
        PropPair { .symbol = minwidthSymbol, .value = 0.4f },
      }
    });
    elements.push_back(titleTextbox);
    if (!dockGroupOptions -> collapse()){
      componentsForFields(dockGroupOptions -> configFields, elements);
    }
    return simpleVerticalLayout(elements, glm::vec2(0.f, 0.f), defaultAlignment, glm::vec4(0.f, 0.f, 0.f, 1.f), 0.01f);
  }

  auto dockTextboxNumeric = std::get_if<DockTextboxNumeric>(&config);
  if (dockTextboxNumeric){
    return createDockTextboxNumeric(*dockTextboxNumeric);
  }

  auto dockColorPickerConfig = std::get_if<DockColorPickerConfig>(&config);
  if (dockColorPickerConfig){
    return createDockColorPicker(*dockColorPickerConfig);
  }

  auto dockSelectConfig = std::get_if<DockSelectConfig>(&config);
  if (dockSelectConfig){
    return createDockSelect(*dockSelectConfig);
  }

  modassert(false, "dock component not yet implemented");
  return Component { };
}

void componentsForFields(std::vector<DockConfig>& configFields, std::vector<Component>& elements){
  for (auto &config : configFields){
    auto dockComponent = createDockComponent(config);
    elements.push_back(dockComponent);
  }
}

Component dockFormComponent {
  .draw = [](DrawingTools& drawTools, Props& props){
    DockConfiguration** dockConfigPtr = typeFromProps<DockConfiguration*>(props, valueSymbol);
    modassert(dockConfigPtr, "dockConfigPtr not provided");

    std::vector<Component> elements;
    componentsForFields((*dockConfigPtr) -> configFields, elements);

    Layout layout {
//      .tint = glm::vec4(1.f, 0.f, 0.f, 1.f),
      .tint = styles.secondaryColor,
      .showBackpanel = true,
      .borderColor = styles.highlightColor,
      .minwidth = 0.5f,
      .minheight = 0.f,
      .layoutType = LAYOUT_VERTICAL2, // LAYOUT_VERTICAL2,
      .layoutFlowHorizontal = UILayoutFlowNegative2, // L UILayoutFlowNone2,
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
        { .symbol = layoutSymbol, .value = layout },
      },
    };

    auto boundingBox = withProps(layoutComponent, listLayoutProps).draw(drawTools, props);
    //drawDebugBoundingBox(drawTools, cameraBoundingBox, glm::vec4(1.f, 0.f, 0.f, 1.f));
    return boundingBox;
  },
};


Component dockComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    std::vector<Component> elements;
    auto dockType = strFromProp(props, dockTypeSymbol, "");
    DockConfiguration* dockConfig = dockConfigByName(dockType);
    modassert(dockConfig, std::string("dock config is null for: " + dockType));

    Props dockProps {
      .props = {
        PropPair { .symbol = valueSymbol, .value = dockConfig },
      }
    };


    elements.push_back(withProps(dockFormComponent, dockProps));

    // this interpolation (for animation) shouldn't be happening int he componnet here
    float xoffset = floatFromProp(props, xoffsetSymbol, 1.f);
    float xoffsetFrom = floatFromProp(props, xoffsetFromSymbol, xoffset);
    float interpAmount = floatFromProp(props, interpolationSymbol, 1.f);
    float xoffsetActual = (xoffset * interpAmount) + (xoffsetFrom * (1 - interpAmount));
    float yoffset = floatFromProp(props, yoffsetSymbol, 0.88f);

    Layout layout {
      .tint = styles.primaryColor,
      .showBackpanel = true,
      .borderColor = glm::vec4(1.f, 1.f, 1.f, 0.2f),
      .minwidth = 0.5f,
      .minheight = 0.f,
      .layoutType = LAYOUT_VERTICAL2,
      .layoutFlowHorizontal = UILayoutFlowNegative2,
      .layoutFlowVertical = UILayoutFlowNegative2,
      .alignHorizontal = UILayoutFlowNone2,
      .alignVertical = UILayoutFlowPositive2,
      .spacing = 0.f,
      .minspacing = 0.f,
      .padding = 0.f,
      .shapeOptions = ShapeOptions {  .zIndex = styles.zIndexs.middleLayer },
      .children = elements,
    };

    Props listLayoutProps {
      .props = {
        { .symbol = layoutSymbol, .value = layout },
        { .symbol = xoffsetSymbol, .value = xoffsetActual },
        { .symbol = yoffsetSymbol, .value = yoffset },
      },
    };
    return withProps(layoutComponent, listLayoutProps).draw(drawTools, props);
  },
};




