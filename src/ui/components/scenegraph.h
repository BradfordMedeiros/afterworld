#ifndef MOD_AFTERWORLD_COMPONENTS_SCENEGRAPH
#define MOD_AFTERWORLD_COMPONENTS_SCENEGRAPH

#include "./common.h"
#include "../../global.h"
#include "./listitem.h"
#include "./layout.h"

struct ScenegraphItem {
	objid id;
	std::string label;
	bool expanded;
	std::vector<ScenegraphItem> children;
};

struct Scenegraph {
	std::vector<ScenegraphItem> items;
};

extern Component scenegraphComponent;

Scenegraph createScenegraph();

#endif