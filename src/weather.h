#ifndef MOD_AFTERWORLD_WEATHER
#define MOD_AFTERWORLD_WEATHER

#include "../../ModEngine/src/cscript/cscript_binding.h"
#include "./util.h"

struct Weather {
	std::optional<objid> weatherEmitter;
};

void onWeatherMessage(Weather& weather, std::any& value, objid sceneId);


#endif