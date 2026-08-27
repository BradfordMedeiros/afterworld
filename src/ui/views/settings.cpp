#include "./settings.h"

extern CustomApiBindings* gameapi;

float originalFov = 90.f;
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

void initSettings(){
  getGlobalState().control.invertY = getSaveBoolValue("settings", "invertY", false);
  getGlobalState().control.xsensitivity = getSaveFloatValue("settings", "xsensitivity", 1.f);
  getGlobalState().control.ysensitivity = getSaveFloatValue("settings", "ysensitivity", 1.f);
}


Component settingsComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    std::vector<Component> elements;
    //elements.push_back(menuList);
    //elements.push_back(settingsInner);

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