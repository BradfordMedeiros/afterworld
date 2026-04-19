#ifndef MOD_AFTERWORLD_COMPONENTS_PLAYING
#define MOD_AFTERWORLD_COMPONENTS_PLAYING

#include "../components/common.h"
#include "../components/game/hud.h"
#include "../components/game/zoom.h"
#include "../components/game/score.h"
#include "../components/game/terminal.h"
#include "./pausemenu.h"
#include "./ball.h"
#include "./mainmenu2.h"

struct UiModeNone{};
struct FpsModeUi {};
struct BallModeUi {};
typedef std::variant<UiModeNone, FpsModeUi, BallModeUi> UiMode;
void changeUiMode(UiMode);

struct PlayingOptions {
	bool showHud;
	std::optional<ZoomOptions> showZoomOverlay;
	std::optional<ScoreOptions> scoreOptions;
	std::optional<TerminalConfig> terminalConfig;
	std::optional<BallComponentOptions> ballMode;

	std::optional<MainMenu2Options> menuOptions;
	bool showGameOver;
	bool showPause;
};


extern Component playingComponent;

#endif

