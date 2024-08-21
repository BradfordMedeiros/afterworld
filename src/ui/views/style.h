#ifndef MOD_AFTERWORLD_COMPONENTS_STYLE
#define MOD_AFTERWORLD_COMPONENTS_STYLE

#include "../components/common.h"

// grep -r "STYLE_UI" for elements that should probably be pulled into here

struct ZIndexs {
	int lowLayer;
	int middleLayer;
	int topLayer;
};

struct Styles {
	ZIndexs zIndexs;
	glm::vec4 primaryColor;
	glm::vec4 secondaryColor;
	glm::vec4 thirdColor;
	glm::vec4 highlightColor;
	glm::vec4 debugColor;
	glm::vec4 debugColor2;
	glm::vec4 mainBorderColor;
	float dockElementPadding;
};

void initStyles();

void setPrimaryColor(glm::vec4 color);
void setSecondaryColor(glm::vec4 color);
void setMainBorderColor(glm::vec4 color);
void setHighlightColor(glm::vec4 color);
void resetColors();

extern Styles styles;

#endif

