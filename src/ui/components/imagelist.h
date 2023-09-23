#ifndef MOD_AFTERWORLD_COMPONENTS_IMAGELIST
#define MOD_AFTERWORLD_COMPONENTS_IMAGELIST

#include "./common.h"

extern const int imagesSymbol;

struct ImageListImage {
	std::string image;
	std::optional<glm::vec4> tint;
};

struct ImageList {
 	std::vector<ImageListImage> images;
};

extern Component imageList;

#endif