#ifndef MOD_AFTERWORLD_COMPONENTS_LAYOUT
#define MOD_AFTERWORLD_COMPONENTS_LAYOUT

#include "../common.h"

extern CustomApiBindings* gameapi;


enum UILayoutType2 { LAYOUT_HORIZONTAL2, LAYOUT_VERTICAL2 };
enum UILayoutFlowType2 { UILayoutFlowNone2, UILayoutFlowNegative2, UILayoutFlowPositive2 };

struct Layout {
	glm::vec4 tint;
	bool showBackpanel;
	std::optional<glm::vec4> borderColor;
	float minwidth;
  float minheight;
  UILayoutType2 layoutType;
  UILayoutFlowType2 layoutFlowHorizontal;
  UILayoutFlowType2 layoutFlowVertical;
  UILayoutFlowType2 alignHorizontal;
  UILayoutFlowType2 alignVertical;
  float spacing;
  float minspacing;
  std::optional<float> padding;
  std::optional<ShapeOptions> shapeOptions;

  std::vector<Component> children;
};

extern Component layoutComponent;

#endif