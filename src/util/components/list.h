#ifndef MOD_AFTERWORLD_COMPONENTS_LIST
#define MOD_AFTERWORLD_COMPONENTS_LIST

#include "./common.h"

BoundingBox2D drawImMenuComponentList(DrawingTools& drawTools, std::vector<Component> list, std::optional<objid> mappingId, MenuItemStyle style, float additionalYOffset, float margin);
void processImMouseSelect(std::vector<Component> components, std::optional<objid> mappingId);

#endif