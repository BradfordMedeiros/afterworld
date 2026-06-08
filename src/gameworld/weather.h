#ifndef MOD_AFTERWORLD_WEATHER
#define MOD_AFTERWORLD_WEATHER

#include "../../../ModEngine/src/cscript/cscript_binding.h"
#include "../util.h"

struct Weather {
	std::optional<objid> weatherEmitter;
};

void changeWeather(Weather& weather, std::optional<std::string> name);
void onWeatherFrame(Weather& weather);

#endif