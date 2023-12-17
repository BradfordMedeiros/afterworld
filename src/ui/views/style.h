#ifndef MOD_AFTERWORLD_COMPONENTS_STYLE
#define MOD_AFTERWORLD_COMPONENTS_STYLE

#include "../components/common.h"

// grep -r "STYLE_UI" for elements that should probably be pulled into here

struct Styles {
	glm::vec4 primaryColor;
	glm::vec4 highlightColor;
	glm::vec4 debugColor;
	glm::vec4 mainBorderColor;

};

extern Styles styles;

#endif

