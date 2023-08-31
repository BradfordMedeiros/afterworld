#ifndef MOD_AFTERWORLD_COMPONENTS_LAYOUT
#define MOD_AFTERWORLD_COMPONENTS_LAYOUT

#include "./common.h"

extern CustomApiBindings* gameapi;


//int limit;
//float limitsize;
//enum LayoutContentAlignmentType { LayoutContentAlignment_Negative, LayoutContentAlignment_Neutral, LayoutContentAlignment_Positive };
//struct LayoutContentAlignment {
//  LayoutContentAlignmentType vertical;
//  LayoutContentAlignmentType horizontal;
//
//enum LayoutContentSpacing  { LayoutContentSpacing_Pack, LayoutContentSpacing_SpaceForFirst, LayoutContentSpacing_SpaceForLast };


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

  std::vector<Component> children;
};

extern Component layoutComponent;

Component simpleVerticalLayout(std::vector<Component>& children);
Component simpleHorizontalLayout(std::vector<Component>& children);

#endif