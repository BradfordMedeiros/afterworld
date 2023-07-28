#ifndef MOD_AFTERWORLD_COMPONENTS_LAYOUT
#define MOD_AFTERWORLD_COMPONENTS_LAYOUT

#include "./common.h"

extern CustomApiBindings* gameapi;


enum UILayoutType { LAYOUT_HORIZONTAL, LAYOUT_VERTICAL };

struct Layout {
	glm::vec4 tint;
	bool showBackpanel;
	std::optional<glm::vec4> borderColor;
	float minwidth;
  float minheight;
  UILayoutType layoutType;
  float spacing;
  float minspacing;
  std::optional<float> margin;


  std::vector<Component> children;
};

Component createLayoutComponent(Layout& layout);

#endif