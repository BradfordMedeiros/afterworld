#include "./style.h"

const bool DEBUG_STYLES = false;

Styles style1 {
	.primaryColor = glm::vec4(0.f, 0.f, 0.f, 0.5f),
	.secondaryColor = DEBUG_STYLES ? glm::vec4(0.f, 0.f, 1.f, 1.f) : glm::vec4(0.2f, 0.2f, 0.2f, 0.5f),
	.thirdColor = DEBUG_STYLES ? glm::vec4(0.f, 1.f, 1.f, 1.f) : glm::vec4(0.4f, 0.4f, 0.4f, 0.5f),
	.highlightColor = glm::vec4(0.f, 1.f, 0.f, 0.5f),
	.debugColor = DEBUG_STYLES ? glm::vec4(0.f, 0.f, 0.f, 1.f) : glm::vec4(0.f, 0.f, 0.f, 0.f),
	.debugColor2 = DEBUG_STYLES ? glm::vec4(0.f, 0.f, 0.f, 1.f) : glm::vec4(0.f, 0.f, 0.f, 0.f),
	.mainBorderColor = glm::vec4(0.f, 0.f, 0.f, 1.f),
	.dockElementPadding = 0.02f,
};

Styles style2 {
	.primaryColor = glm::vec4(1.f, 0.f, 0.f, 1.f),
	.secondaryColor = glm::vec4(1.f, 0.7f, 0.f, 0.5f),
	.thirdColor = glm::vec4(0.4f, 0.4f, 0.4f, 0.5f),
	.highlightColor = glm::vec4(1.f, 1.f, 0.5f, 0.5f),
	.debugColor = glm::vec4(0.f, 0.f, 0.f, 0.f),
	.debugColor2 = glm::vec4(0.f, 0.f, 0.f, 0.f),
	.mainBorderColor = glm::vec4(0.f, 0.f, 0.f, 1.f),
	.dockElementPadding = 0.02f,
};

Styles styles = style1;


