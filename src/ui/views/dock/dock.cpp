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


int currentDebugMask(){
  auto value = dockConfigApi.getAttribute("editor", "debugmask");
  auto debugMaskFloat = std::get_if<float>(&value);
  modassert(debugMaskFloat, "debug mask is not a float");
  auto debugValue = static_cast<int>(*debugMaskFloat);
  return debugValue; 
}
std::function<bool()> getIsDebugMaskEnabled(int bitmask){
  return [bitmask]() -> bool {
    auto debugValue = currentDebugMask();
    auto newMask = debugValue & bitmask;
    //std::cout << "debug value is: " << debugValue << ", check against: " << bitmask << ", new mask: " << newMask << std::endl;
    return newMask != 0;
  };
}

std::function<void(bool)> getOnDebugMaskEnabled(int bitmask){
  return [bitmask](bool checked) -> void {
    auto debugMask = currentDebugMask();
    if (checked){
      debugMask = debugMask | bitmask; // add the bit to the old mask
    }else{
      debugMask = debugMask & ~(bitmask);   // take the bit away from the old mask, by making all 1s except the bit
    }
    auto floatMask = static_cast<float>(debugMask);
    dockConfigApi.setAttribute("editor", "debugmask", floatMask);
  };
}

std::unordered_map<std::string, bool> collapseValues;
std::function<void()> createCollapsableOnClick(const char* value){
  collapseValues[value] = true;
  return [value]() -> void { collapseValues[value] = !collapseValues.at(value); };
}
std::function<bool()> createShouldBeCollapse(const char* value){
  return [value]() -> bool { return collapseValues.at(value); };
}

std::map<std::string, std::string> textStore;
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
  DockConfiguration {
    .title = "Object Details",
    .configFields = {
      DockTextboxNumeric {
        .label = "position x",
        .value = []() -> std::string{ return "1.0"; },
        .onEdit = [](float, std::string&) -> void { },
      },
      DockTextboxNumeric {
        .label = "position y",
        .value = []() -> std::string{ return "1.0"; },
        .onEdit = [](float, std::string&) -> void { },
      },
      DockTextboxNumeric {
        .label = "position z",
        .value = []() -> std::string{ return "1.0"; },
        .onEdit = [](float, std::string&) -> void { },
      },
      DockTextboxNumeric {
        .label = "scale x",
        .value = []() -> std::string{ return "1.0"; },
        .onEdit = [](float, std::string&) -> void { },
      },
      DockTextboxNumeric {
        .label = "scale y",
        .value = []() -> std::string{ return "1.0"; },
        .onEdit = [](float, std::string&) -> void { },
      },
      DockTextboxNumeric {
        .label = "scale z",
        .value = []() -> std::string{ return "1.0"; },
        .onEdit = [](float, std::string&) -> void { },
      },
    },
  },
  DockConfiguration {
    .title = "Textures",
    .configFields = {
      DockTextboxConfig {
        .label = "Tiling",
        .text = connectGetTextVec2("texturetiling"),
        .onEdit = connectEditTextVec2("tiling-x", "texturetiling"),
      },
      DockTextboxConfig {
        .label = "Size",
        .text = connectGetTextVec2("texturesize"),
        .onEdit = connectEditTextVec2("texturesize", "texturesize"),
      },
      DockTextboxConfig {
        .label = "Offset",
        .text = connectGetTextVec2("textureoffset"),
        .onEdit = connectEditTextVec2("textureoffset", "textureoffset"),
      },
      DockImageConfig {
        .label =  "someimage-here",
        .onImageSelect = [](std::string texture) -> void {
          modlog("dock set texture", texture);
          dockConfigApi.setTexture(texture);
        }
      },
    },
  },
  DockConfiguration {
    .title = "Editor",
    .configFields = {
      DockCheckboxConfig {
        .label = "Group Selection",
        .isChecked = getIsCheckedWorld("editor", "groupselection", "true", "false"),
        .onChecked = getOnCheckedWorld("editor", "groupselection", "true", "false"),
      },
      DockCheckboxConfig {
        .label = "Symmetric Translate",
        .isChecked = getIsCheckedWorld("tools", "position-mirror", "true", "false"),
        .onChecked = getOnCheckedWorld("tools", "position-mirror", "true", "false"),
      },
      DockCheckboxConfig {
        .label = "Absolute Translate",
        .isChecked = getIsCheckedWorld("tools", "snap-position", "absolute", "relative"),
        .onChecked = getOnCheckedWorld("tools", "snap-position", "absolute", "relative"),
      },
      DockOptionConfig { // Snap Translates
        .options = { "0.01", "0.1", "0.5", "1", "5" },
        .onClick = optionsOnClick("editor", "snaptranslate-index", { 0.0f, 1.0f, 2.0f, 3.0f, 4.0f }),
        .getSelectedIndex = optionsSelectedIndex("editor", "snaptranslate-index", { 0.0f, 1.0f, 2.0f, 3.0f, 4.0f }),
      },
      DockCheckboxConfig {
        .label = "Preserve Scale",
        .isChecked = getIsCheckedWorld("tools", "preserve-scale", "true", "false"),
        .onChecked = getOnCheckedWorld("tools", "preserve-scale", "true", "false"),
      },
      DockOptionConfig {  // "Snap Scales",
        .options = { "0.01", "0.1", "0.5", "1", "5" },
        .onClick = optionsOnClick("editor", "snapscale-index", { 0.0f, 1.0f, 2.0f, 3.0f, 4.0f }),
        .getSelectedIndex = optionsSelectedIndex("editor", "snapscale-index", { 0.0f, 1.0f, 2.0f, 3.0f, 4.0f }),
      },
      DockCheckboxConfig {
        .label = "Absolute Rotation",
        .isChecked = getIsCheckedWorld("tools", "snap-rotate", "absolute", "relative"),
        .onChecked = getOnCheckedWorld("tools", "snap-rotate", "absolute", "relative"),
      },
      DockOptionConfig { // Snap Rotation
        .options = { "1", "5", "15", "30", "45", "90", "180" },
        .onClick = optionsOnClick("editor", "snapangle-index", { 0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f }),
        .getSelectedIndex = optionsSelectedIndex("editor", "snapangle-index", { 0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f }),
      },
    },
  },
  DockConfiguration {
    .title = "Cameras",
    .configFields = {
      DockButtonConfig {
        .buttonText = "Create Camera",
        .onClick = []() -> void { dockConfigApi.createCamera(); },
      },
      DockOptionConfig {
        .options = { "enable dof", "disable dof" },
        .onClick = optionsOnClickObj("dof", { "enabled", "disabled" }),
        .getSelectedIndex = optionsSelectedIndexObj("dof",  { "enabled", "disabled" }),
      },
      DockCheckboxConfig {
        .label = "enable physics",
        .isChecked = getIsCheckedGameobj("physics", "enabled", "disabled"),
        .onChecked = getOnCheckedGameobj("physics", "enabled", "disabled"),
      },    

      DockGroup {
        .groupName = "Depth of Field Blur",
        .onClick = createCollapsableOnClick("blur"),
        .collapse = createShouldBeCollapse("blur"),
        .configFields = {
          DockGameObjSelector {
            .label = std::nullopt,
            .onSelect =  getStrGameObj("target"),
          },
          DockCheckboxConfig {
            .label = "toggle dof",
            .isChecked = getIsCheckedGameobj("dof", "enabled", "disabled"),
            .onChecked = getOnCheckedGameobj("dof", "enabled", "disabled"),
          },
          DockSliderConfig {
            .label = "blur min",
            .min = 0.f,
            .max = 1.f,
            .percentage = getFloatGameobj("minblur"),
            .onSlide = [](float amount) -> void {
              auto setMinBlur = getSetFloatGameobj("minblur");
              setMinBlur(amount);
              modassert(false, std::string("set min blur, value = " ) + std::to_string(amount));
            },
          },
          DockSliderConfig {
            .label = "blur max",
            .min = 0.f,
            .max = 1.f,
            .percentage = getFloatGameobj("maxblur"),
            .onSlide = getSetFloatGameobj("maxblur"),
          },
          DockSliderConfig {
            .label = "blur amount",
            .min = 0.f,
            .max = 1.f,
            .percentage = getFloatGameobj("bluramount"),
            .onSlide = getSetFloatGameobj("bluramount"),
          },
        },
      },
    },
  },
  DockConfiguration {
    .title = "Lights",
    .configFields = {
      DockButtonConfig {
        .buttonText = "Create Light",
        .onClick = []() -> void { dockConfigApi.createLight(); } ,
      },
      DockOptionConfig {
        .options = { "point", "spotlight", "directional" },
        .onClick = optionsOnClickObj("type", { "point", "spotlight", "directional" }),
        .getSelectedIndex = optionsSelectedIndexObj("type", { "point", "spotlight", "directional" }),
      },
      DockTextboxConfig {
        .label = "Test Vec2",
        .text = connectGetText("test-text"),
        .onEdit = connectEditText("test-text", TEXT_TYPE_VEC2 ),
      },
      DockTextboxConfig {
        .label = "Test Vec",
        .text = connectGetText("test-text"),
        .onEdit = connectEditText("test-text"),
      },
    },
  },
  DockConfiguration {
    .title = "Scenegraph",
    .configFields = {
      DockScenegraph {},
    },
  },
  DockConfiguration {
    .title = "Debug",
    .configFields = {
      DockCheckboxConfig {
        .label = "Show Debug",
        .isChecked = getIsCheckedWorld("editor", "debug", "true", "false"),
        .onChecked = getOnCheckedWorld("editor", "debug", "true", "false"),
      },
      DockCheckboxConfig {
        .label = "Show Cameras",
        .isChecked = getIsDebugMaskEnabled(0b10),
        .onChecked = getOnDebugMaskEnabled(0b10),
      },
      DockCheckboxConfig {
        .label = "Show Lights",
        .isChecked = getIsDebugMaskEnabled(0b1000),
        .onChecked = getOnDebugMaskEnabled(0b1000),
      },
      DockCheckboxConfig {
        .label = "Show Sound",
        .isChecked = getIsDebugMaskEnabled(0b100),
        .onChecked = getOnDebugMaskEnabled(0b100),
      },
      DockCheckboxConfig {
        .label = "Show Emitters",
        .isChecked = getIsDebugMaskEnabled(0b100000),
        .onChecked = getOnDebugMaskEnabled(0b100000),
      },   
    },
  },
  DockConfiguration {
    .title = "Transform",
    .configFields = {
      DockOptionConfig { // Snap Translates
        .options = { "translate", "scale", "rotate" },
        .onClick = optionsOnClick("tools", "manipulator-mode", { "translate", "scale", "rotate" }),
        .getSelectedIndex = optionsSelectedIndex("tools", "manipulator-mode", { "translate", "scale", "rotate" }),
      },
      DockOptionConfig { // Snap Translates
        .options = { "x", "y", "z" },
        .onClick = optionsOnClick("tools", "manipulator-axis", { "x", "y", "z" }),
        .getSelectedIndex = optionsSelectedIndex("tools", "manipulator-axis", { "x", "y", "z" }),
      },
      DockOptionConfig { 
        .options = { "+x", "-x", "+y", "-y", "-z", "+z" },
        .onClick = [](std::string& choice, int) -> void {
          // this should use the manipulator api, so it can orient on the axises
          if (choice == "+x"){
            sendManipulatorEvent(OBJECT_ORIENT_RIGHT);
          }else if (choice == "-x"){
            sendManipulatorEvent(OBJECT_ORIENT_LEFT);
          }else if (choice == "+y"){
            sendManipulatorEvent(OBJECT_ORIENT_UP);
          }else if (choice == "-y"){
            sendManipulatorEvent(OBJECT_ORIENT_DOWN);
          }else if (choice == "+z"){
            sendManipulatorEvent(OBJECT_ORIENT_BACK);
          }else if (choice == "-z"){
            sendManipulatorEvent(OBJECT_ORIENT_FORWARD);
          }
        },
        .getSelectedIndex = []() -> int {
          return -1;
        },
      },
    }
  },
  DockConfiguration {
    .title = "Navmesh",
    .configFields = {
      DockButtonConfig {
        .buttonText = "Create Navmesh",
        .onClick = []() -> void { dockConfigApi.createNavmesh(); } ,
      },
    }
  },

  // Gameplay Docks //////////////////////////////////////////////
  //////////////////////////////////////////////////////////////
  DockConfiguration {
    .title = "MOVEMENT",
    .configFields = {
      DockButtonConfig {
        .buttonText = "MOVEMENT",
        .onClick = []() -> void {},
      },
      createSimpleTextboxNumeric("traits", "Speed", "speed", []() -> std::optional<SqlFilter> { return SqlFilter { .column = "profile", .value = "default" }; }),
      createSimpleTextboxNumeric("traits", "Speed Air", "speed-air", []() -> std::optional<SqlFilter> { return SqlFilter { .column = "profile", .value = "default" }; }),
      createSimpleTextboxNumeric("traits", "Jump Height", "jump-height", []() -> std::optional<SqlFilter> { return SqlFilter { .column = "profile", .value = "default" }; }),
      createSimpleTextboxNumeric("traits", "Gravity", "gravity", []() -> std::optional<SqlFilter> { return SqlFilter { .column = "profile", .value = "default" }; }),
      createSimpleTextboxNumeric("traits", "Mass", "mass", []() -> std::optional<SqlFilter> { return SqlFilter { .column = "profile", .value = "default" }; }),
      createSimpleTextboxNumeric("traits", "Friction", "friction", []() -> std::optional<SqlFilter> { return SqlFilter { .column = "profile", .value = "default" }; }),
      createSimpleTextboxNumeric("traits", "Restitution", "restitution", []() -> std::optional<SqlFilter> { return SqlFilter { .column = "profile", .value = "default" }; }),
      createSimpleCheckbox("traits", "Crouch", "crouch", []() -> SqlFilter { return SqlFilter { .column = "profile", .value = "default" }; }),
      createSimpleCheckbox("traits", "Move Vertical", "move-vertical", []() -> SqlFilter { return SqlFilter { .column = "profile", .value = "default" }; }),
    }
  },
  DockConfiguration {
    .title = "WEAPONS",
    .configFields = {
      DockSelectConfig {
        .selectOptions = SelectOptions {
          .getOptions = []() -> std::vector<std::string>& {
            static std::vector<std::string> options = listGuns();
            return options;
          },
          .toggleExpanded = [](bool expanded) -> void {
            weaponsExpanded = expanded;
          },
          .onSelect = [](int index, std::string& gun) -> void {
            weaponSelectIndex = index;
            weaponsExpanded = false;
            selectedGun = gun;
            modlog("editor gun", std::string("selected gun: ") + gun);
          },
          .currentSelection = []() -> int { return weaponSelectIndex; },
          .isExpanded = []() -> bool { return weaponsExpanded; },
        }
      },
      createSimpleGunCheckbox("Ironsight", "ironsight"),
      createSimpleGunCheckbox("Raycast", "raycast"),
      createSimpleGunCheckbox("Hold", "hold"),
      createSimpleTextboxNumeric("guns","Bloom", "bloom"),
      createSimpleTextboxNumeric("guns","Min Bloom", "minbloom"),
      createSimpleTextboxNumeric("guns","Bloom Length", "bloom-length"),
      createSimpleTextboxNumeric("guns","Horizontal Sway", "bloom-length"),
      createSimpleTextboxNumeric("guns","Vertical Sway", "bloom-length"),
    }
  },
  DockConfiguration {
    .title = "TRIGGERS",
    .configFields = {
      DockButtonConfig {
        .buttonText = "Create Trigger",
        .onClick = []() -> void {},
      },
      DockTextboxConfig {
        .label = "Trigger",
        .text = []() -> std::string {  // gameobj:trigger-switch
          return "trigger-name-plaeholder"; 
        },
        .onEdit = [](std::string value) -> void {

        }
      },
    }
  },
  DockConfiguration {
    .title = "HUD",
    .configFields = {
      DockButtonConfig {
        .buttonText = "HUD",
        .onClick = []() -> void {},
      },
    }
  },
  DockConfiguration {
    .title = "WATER",
    .configFields = {
      DockButtonConfig {
        .buttonText = "Create Water",
        .onClick = []() -> void {},
      },
      DockTextboxNumeric {
        .label = "Density",
        .value = []() -> std::string { return "1.0"; },
        .onEdit = [](float, std::string&) -> void { },
        // gameobj:water-density   // positive number
      },
      DockTextboxNumeric {
        .label = "Viscosity",
        .value = []() -> std::string { return "1.0"; },
        .onEdit = [](float, std::string&) -> void { },
        // gameobj:water-viscosity  // positive number
      },
      DockTextboxNumeric {
        .label = "Gravity",
        .value = []() -> std::string { return "1.0"; },
        .onEdit = [](float, std::string&) -> void {  },
        // gameobj:water-gravity  // positive number
      },
    }
  },


  //// Editor Docks ////////////////
  /////////////////////////////////////
  DockConfiguration {
    .title = "COLORS",
    .configFields = {
      DockColorPickerConfig {
        .label = "primary",
        .getColor = []() -> glm::vec4 { return styles.primaryColor; },
        .onColor = [](glm::vec4 color) -> void {
          setPrimaryColor(color);
        },
      },
      DockColorPickerConfig {
        .label = "secondary",
        .getColor = []() -> glm::vec4 { return styles.secondaryColor; },
        .onColor = [](glm::vec4 color) -> void { 
          setSecondaryColor(color);
        },
      },
      DockColorPickerConfig {
        .label = "border",
        .getColor = []() -> glm::vec4 { return styles.mainBorderColor; },
        .onColor = [](glm::vec4 color) -> void { 
          setMainBorderColor(color);
        },
      },
      DockColorPickerConfig {
        .label = "highlight",
        .getColor = []() -> glm::vec4 { return styles.highlightColor; },
        .onColor = [](glm::vec4 color) -> void { 
          setHighlightColor(color);
        },
      },
      DockImageConfig {
        .label =  "[unset]",
        .onImageSelect = [](std::string texture) -> void {
          dockConfigApi.setEditorBackground(texture);
        }
      },
      DockButtonConfig {
        .buttonText = "Reset",
        .onClick = []() -> void {
          resetColors();
        },
      },
      // then a background picker 
    }
  },

  DockConfiguration {
    .title = "Particle Viewer",
    .configFields = {
      DockButtonConfig {
        .buttonText = "Emit One",
        .onClick = []() -> void {
          dockConfigApi.emitParticleViewerParticle();
        },
      },
      DockCheckboxConfig {
        .label = "Emit Particles",
        .isChecked = []() -> bool {
          return dockConfigApi.getParticlesViewerShouldEmit();
        },
        .onChecked = [](bool isChecked) -> void { 
          dockConfigApi.setParticlesViewerShouldEmit(isChecked);
        },
      },
      DockTextboxNumeric {
        .label = "rate",
        .value = floatParticleGetValue("rate"),
        .onEdit = floatParticleSetValue("rate"),
      },
      DockTextboxNumeric {
        .label = "duration",
        .value = floatParticleGetValue("duration"),
        .onEdit = floatParticleSetValue("duration"),
      },
      DockTextboxNumeric {
        .label = "limit",
        .value = floatParticleGetValue("limit"),
        .onEdit = floatParticleSetValue("limit"),
      },
      DockGroup {
        .groupName = "Base Particle",
        .onClick = createCollapsableOnClick("particle-base"),
        .collapse = createShouldBeCollapse("particle-base"),
        .configFields = {
          DockCheckboxConfig {
            .label = "enable physics",
            .isChecked = floatParticleGetValueBool("+physics", "enabled", "disabled"),
            .onChecked = floatParticleSetValueBool("+physics", "enabled", "disabled"),
          },    
          DockCheckboxConfig {
            .label = "enable collision",
            .isChecked = floatParticleGetValueBool("+physics_collision", "collide", "nocollide"),
            .onChecked = floatParticleSetValueBool("+physics_collision", "collide", "nocollide"),
          },    
          DockColorPickerConfig {
            .label = "tint",
            .getColor = []() -> glm::vec4 { 
              auto attr = dockConfigApi.getParticleAttribute("+tint");
              if (!attr.has_value()){
                return glm::vec4(0.f, 0.f, 0.f, 0.f);
              }
              auto vec4Value = std::get_if<glm::vec4>(&attr.value());
              modassert(vec4Value, "has tint but not a vec4");
              return *vec4Value;
            },
            .onColor = [](glm::vec4 color) -> void {
              dockConfigApi.setParticleAttribute("+tint", color);
            },
          },
          DockImageConfig {
            .label =  "texture",
            .onImageSelect = [](std::string texture) -> void {
              dockConfigApi.setParticleAttribute("+texture", texture);
            }
          },
          DockTextboxNumeric {
            .label = "gravity-x",
            .value = floatParticleGetValueVec3("+physics_gravity", 0),
            .onEdit = floatParticleSetValueVec3("+physics_gravity", 0),
          },
          DockTextboxNumeric {
            .label = "gravity-y",
            .value = floatParticleGetValueVec3("+physics_gravity", 1),
            .onEdit = floatParticleSetValueVec3("+physics_gravity", 1),
          },
          DockTextboxNumeric {
            .label = "gravity-z",
            .value = floatParticleGetValueVec3("+physics_gravity", 2),
            .onEdit = floatParticleSetValueVec3("+physics_gravity", 2),
          },
          DockTextboxNumeric {
            .label = "velocity-x",
            .value = floatParticleGetValueVec3("+physics_velocity", 0),
            .onEdit = floatParticleSetValueVec3("+physics_velocity", 0),
          },
          DockTextboxNumeric {
            .label = "velocity-y",
            .value = floatParticleGetValueVec3("+physics_velocity", 1),
            .onEdit = floatParticleSetValueVec3("+physics_velocity", 1),
          },
          DockTextboxNumeric {
            .label = "velocity-z",
            .value = floatParticleGetValueVec3("+physics_velocity", 2),
            .onEdit = floatParticleSetValueVec3("+physics_velocity", 2),
          },
          DockTextboxNumeric {
            .label = "scale-x",
            .value = floatParticleGetValueVec3("+scale", 0),
            .onEdit = floatParticleSetValueVec3("+scale", 0),
          },
          DockTextboxNumeric {
            .label = "scale-y",
            .value = floatParticleGetValueVec3("+scale", 1),
            .onEdit = floatParticleSetValueVec3("+scale", 1),
          },
          DockTextboxNumeric {
            .label = "scale-z",
            .value = floatParticleGetValueVec3("+scale", 2),
            .onEdit = floatParticleSetValueVec3("+scale", 2),
          },
          /*DockCheckboxConfig {
            .label = "Billboard",
            .isChecked = getIsCheckedWorld("tools", "position-mirror", "true", "false"),
            .onChecked = getOnCheckedWorld("tools", "position-mirror", "true", "false"),
          },
          DockFileConfig {
            .label = "model-here",
            .displayLimit = 30,
          },*/
        },
      },
      DockGroup {
        .groupName = "Particle Values",
        .onClick = createCollapsableOnClick("particle-values"),
        .collapse = createShouldBeCollapse("particle-values"),
        .configFields = {
          DockTextboxNumeric {
            .label = "position-x",
            .value = floatParticleGetValueVec3("!position", 0),
            .onEdit = floatParticleSetValueVec3("!position", 0),
          },
          DockTextboxNumeric {
            .label = "position-y",
            .value = floatParticleGetValueVec3("!position", 1),
            .onEdit = floatParticleSetValueVec3("!position", 1),
          },
          DockTextboxNumeric {
            .label = "position-z",
            .value = floatParticleGetValueVec3("!position", 2),
            .onEdit = floatParticleSetValueVec3("!position", 2),
          },
          DockTextboxNumeric {
            .label = "scale-x",
            .value = floatParticleGetValueVec3("!scale", 0),
            .onEdit = floatParticleSetValueVec3("!scale", 0),
          },
          DockTextboxNumeric {
            .label = "scale-y",
            .value = floatParticleGetValueVec3("!scale", 1),
            .onEdit = floatParticleSetValueVec3("!scale", 1),
          },
          DockTextboxNumeric {
            .label = "scale-z",
            .value = floatParticleGetValueVec3("!scale", 2),
            .onEdit = floatParticleSetValueVec3("!scale", 2),
          },
        }
      }, 
      DockGroup {
        .groupName = "Particle Variance",
        .onClick = createCollapsableOnClick("particle-variance"),
        .collapse = createShouldBeCollapse("particle-variance"),
        .configFields = {
          DockTextboxNumeric {
            .label = "position",
            .value = []() -> std::string{ return "1.0"; },
            .onEdit = [](float, std::string&) -> void { },
          },
          DockTextboxNumeric {
            .label = "scale",
            .value = []() -> std::string{ return "1.0"; },
            .onEdit = [](float, std::string&) -> void { },
          },
        }
      }
    }
  },

  DockConfiguration {
    .title = "Models",
    .configFields = {
      DockButtonConfig {
        .buttonText = "Create Model",
        .onClick = []() -> void {},
      },
      DockFileConfig {
        .label = "Select Model",
        .displayLimit = 30,
        .onFileSelected = [](std::string& file) -> void {
          // TODO REALFILES
          std::filesystem::path workingDir = std::filesystem::current_path(); 
          std::filesystem::path absolutePath = file;
          std::filesystem::path relativePath = std::filesystem::relative(absolutePath, workingDir);
          auto pathAsStr = relativePath.string();
          std::string meshName = "../gameresources/build/primitives/loop.gltf";
          dockConfigApi.createMesh(pathAsStr);
        },
        .filterFilter = [](bool isDirectory, std::string& file) -> bool {
          if (isDirectory){
            return true;
          }
          auto extension = getExtension(file);
          if (extension.has_value()){
            if (extension.value() == "gltf"){
              return true;
            }else if (extension.value() == "obj"){
              return true;
            }else if (extension.value() == "fbx"){
              return true;
            }
          }
          return false;
        },
      },
    }
  },
  DockConfiguration {
    .title = "Scene",
    .configFields = {
      DockButtonConfig {
        .buttonText = "Save Scene",
        .onClick = []() -> void {
          dockConfigApi.saveScene();
        },
      },
      DockButtonConfig {
        .buttonText = "Reset Scene",
        .onClick = []() -> void {
          dockConfigApi.resetScene();
        },
      },
    }
  },

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

  auto fileconfigOptions = std::get_if<DockFileConfig>(&config);
  if (fileconfigOptions){
    return createDockFile(*fileconfigOptions);
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

  auto dockScenegraphOptions = std::get_if<DockScenegraph>(&config);
  if (dockScenegraphOptions){
    return scenegraphContainer;
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




