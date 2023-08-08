#ifndef MOD_AFTERWORLD_COMPONENTS_IMAGELIST
#define MOD_AFTERWORLD_COMPONENTS_IMAGELIST

#include "./common.h"
#include "../../global.h"

struct ImageList {
 	std::vector<std::string> images;
};

Component createImageList();

#endif