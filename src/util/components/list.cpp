#include "./list.h"

Component createList(std::vector<ListComponentData> listItems){
  std::vector<Component> elements;
  for (auto &listItem : listItems){
    if (listItem.onClick.has_value()){
      elements.push_back(createListItem(listItem.name, listItem.onClick.value()));
    }else{
      elements.push_back(createListItem(listItem.name));
    }
  }
  Layout layout {
    .tint = glm::vec4(0.f, 0.f, 0.f, 0.8f),
    .showBackpanel = false,
    .borderColor = glm::vec4(0.2f, 0.2f, 0.2f, 1.f),
    .minwidth = 0.f,
    .minheight = 0.f,
    .layoutType = LAYOUT_VERTICAL2,
    .layoutFlowHorizontal = UILayoutFlowNone2,
    .layoutFlowVertical = UILayoutFlowNone2,
    .alignHorizontal = UILayoutFlowNone2,
    .alignVertical = UILayoutFlowNone2,
    .spacing = 0.f,
    .minspacing = 0.f,
    .padding = 0.f,
    .children = elements,
  };

  return createLayoutComponent(layout);
}

const int tintSymbol = getSymbol("listitems");

Component listComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    //auto imageList = typeFromProps<ImageList>(props, imagesSymbol);
    //modassert(imageList, "invalid image list prop");

  },
  .imMouseSelect = [](std::optional<objid> mappingId, Props& props) -> void {
   
  },
};
