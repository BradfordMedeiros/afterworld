#ifndef MOD_AFTERWORLD_COMPONENTS_PLAYING
#define MOD_AFTERWORLD_COMPONENTS_PLAYING

#include "../components/common.h"
#include "../components/game/hud.h"
#include "../components/game/zoom.h"

struct PlayingOptions {
	bool showHud;
	std::optional<ZoomOptions> showZoomOverlay;
};
extern Component playingComponent;

#endif

