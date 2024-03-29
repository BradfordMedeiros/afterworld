#ifndef MOD_AFTERWORLD_COMPONENTS_LIST_ITEM
#define MOD_AFTERWORLD_COMPONENTS_LIST_ITEM

#include "../common.h"

struct ImListItem {
  std::string value;
  std::optional<std::function<void()>> onClick;
  std::optional<std::function<void(int)>> onClick2;
  std::optional<objid> mappingId;
};

BoundingBox2D drawImMenuList(DrawingTools& drawTools, std::vector<ImListItem> list, float xoffset, float yoffset2, float padding, std::optional<float> fontSizeStyle, float minwidth, float minheight, glm::vec4 tint = glm::vec4(1.f, 1.f, 1.f, .4f), glm::vec4 color = glm::vec4(1.f, 1.f, 1.f, 1.f));

extern Component listItem;

#endif