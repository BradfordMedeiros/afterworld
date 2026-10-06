#ifndef MOD_AFTERWORLD_WEATHER
#define MOD_AFTERWORLD_WEATHER

#include "../../../ModEngine/src/cscript/cscript_binding.h"
#include "../util.h"
#include "../resources/materials.h"

struct Weather {
	std::optional<objid> weatherEmitter;
	std::optional<std::string> weatherName;
	float windStrength = 0.25f;
};

void changeWeather(Weather& weather, std::optional<std::string> name);
void onWeatherFrame(Weather& weather);

#endif