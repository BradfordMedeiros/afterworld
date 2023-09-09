#ifndef MOD_AFTERWORLD_COMPONENTS_IMAGELIST
#define MOD_AFTERWORLD_COMPONENTS_IMAGELIST

#include "./common.h"
#include "../../global.h"

extern const int imagesSymbol;

struct ImageList {
 	std::vector<std::string> images;
};

extern Component imageList;

#endif