#ifndef MOD_AFTERWORLD_COMPONENTS_LIST_ITEM
#define MOD_AFTERWORLD_COMPONENTS_LIST_ITEM

#include "./common.h"

struct ImListItem {
  std::string value;
  std::optional<std::function<void()>> onClick;
  std::optional<objid> mappingId;
};

BoundingBox2D drawImMenuList(DrawingTools& drawTools, std::vector<ImListItem> list, std::optional<objid> mappingId, float xoffset, float yoffset2, float padding, std::optional<float> fontSizeStyle, float minwidth, glm::vec4 tint = glm::vec4(1.f, 1.f, 1.f, .4f), glm::vec4 color = glm::vec4(1.f, 1.f, 1.f, 1.f));
void processImMouseSelect(std::vector<ImListItem> list, std::optional<objid> mappingId);

extern Component listItem;

#endif