#include "./style.h"

extern CustomApiBindings* gameapi;

void initStyles(){
  auto primaryColor = glm::vec4(0.f, 0.f, 0.f, 0.5f);
  auto secondaryColor = glm::vec4(0.2f, 0.2f, 0.2f, 0.5f);
  auto borderColor = glm::vec4(0.f, 0.f, 0.f, 1.f);
  auto highlightColor = glm::vec4(0.f, 1.f, 0.f, 0.5f);

  styles.primaryColor = primaryColor;
  styles.secondaryColor = secondaryColor;
  styles.mainBorderColor = borderColor;
  styles.highlightColor = highlightColor;
}

Styles defaultStyle {
  .zIndexs = {
    .lowLayer = 0,
    .middleLayer = 1,
    .topLayer = 2,
  },
	.primaryColor = glm::vec4(0.f, 0.f, 0.f, 0.5f),
	.secondaryColor = glm::vec4(0.2f, 0.2f, 0.2f, 0.5f),
	.thirdColor = glm::vec4(0.4f, 0.4f, 0.4f, 0.5f),
	.highlightColor = glm::vec4(0.f, 1.f, 0.f, 0.5f),
	.debugColor = glm::vec4(0.f, 0.f, 0.f, 0.f),
	.debugColor2 = glm::vec4(0.f, 0.f, 0.f, 0.f),
	.mainBorderColor = glm::vec4(0.f, 0.f, 0.f, 1.f),
	.dockElementPadding = 0.02f,
};

Styles styles = defaultStyle;

