#ifndef MOD_AFTERWORLD_COMPONENTS_PLAYING
#define MOD_AFTERWORLD_COMPONENTS_PLAYING

#include "../components/common.h"
#include "../components/game/hud.h"
#include "../components/game/zoom.h"
#include "../components/game/terminal.h"
#include "./uicontext.h"
#include "./ball.h"

struct UiModeNone{};
struct FpsModeUi {
	
};
struct BallModeUi {
	BallComponentOptions ballMode;
};

struct MainMenu2Options {
	glm::vec4 backgroundColor;
	float offsetY;

	std::function<void()> onNewGame = []() -> void {};
	std::function<void()> onContinueGame = []() -> void {};
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
};


extern Component playingComponent;

#endif

