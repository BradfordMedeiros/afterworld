#ifndef MOD_AFTERWORLD_COMPONENTS_PLAYING
#define MOD_AFTERWORLD_COMPONENTS_PLAYING

#include "../components/common.h"
#include "../components/game/hud.h"
#include "../components/game/zoom.h"
#include "../components/game/score.h"
#include "../components/game/terminal.h"
#include "./ball.h"
#include "./mainmenu2.h"

struct PlayingOptions {
	bool showHud;
	std::optional<ZoomOptions> showZoomOverlay;
	std::optional<ScoreOptions> scoreOptions;
	std::optional<TerminalConfig> terminalConfig;
	std::optional<BallComponentOptions> ballMode;

	std::optional<MainMenu2Options> menuOptions;


};
extern Component playingComponent;

#endif

