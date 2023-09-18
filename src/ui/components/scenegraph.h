#ifndef MOD_AFTERWORLD_COMPONENTS_SCENEGRAPH
#define MOD_AFTERWORLD_COMPONENTS_SCENEGRAPH

#include "./common.h"
#include "../../global.h"
#include "./listitem.h"
#include "./layout.h"

struct ScenegraphItem {
	objid id;
	std::string label;
	std::vector<ScenegraphItem> children;
};

struct Scenegraph {
	std::vector<ScenegraphItem> items;
	std::set<objid> idToExpanded;
};

extern Component scenegraphComponent;

Scenegraph createScenegraph(std::set<objid> initiallyExpandedIds = {});
void refreshScenegraph(Scenegraph& scenegraph);

#endif