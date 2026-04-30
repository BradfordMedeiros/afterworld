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
struct FpsModeUi {
	
};
struct BallModeUi {
	BallComponentOptions ballMode;
};
struct LiveMenu {
	MainMenu2Options options;
};
struct GameOverUi {};

typedef std::variant<UiModeNone, FpsModeUi, BallModeUi, LiveMenu, GameOverUi> UiMode;
void changeUiMode(UiMode);

std::optional<BallModeUi*> getBallModeUI();
std::optional<LiveMenu*> getLiveMenuUi();

void setTerminalConfig(std::optional<TerminalConfig> terminalConfig);
std::optional<TerminalConfig*> getTerminalConfig();

struct PlayingOptions {
	std::optional<ZoomOptions> showZoomOverlay;
	std::optional<ScoreOptions> scoreOptions;
	std::optional<PauseOptions> pauseOptions;
};


extern Component playingComponent;

#endif

