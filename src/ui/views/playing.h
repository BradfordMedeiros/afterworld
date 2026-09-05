#ifndef MOD_AFTERWORLD_COMPONENTS_PLAYING
#define MOD_AFTERWORLD_COMPONENTS_PLAYING

#include "../components/common.h"
#include "../components/game/hud.h"
#include "../components/game/terminal.h"

struct TerminalConfig;

struct UiModeNone{};
struct FpsModeUi {
	
};

struct BallLevelComplete {};
struct BallComponentOptions {
	std::optional<std::function<float()>> elapsedTime;
	bool showElapsedTime = false;

	std::optional<BallLevelComplete> levelComplete;

	bool showPowerup = false;
	std::optional<std::string> powerupTexture;
	std::optional<float> powerupStartTime;
	std::optional<float> powerupDuration;
};

struct BallLevelSelectInfo {
	std::string world;
	std::string level;
	std::string parTime;
	std::string bestTime;
	int gems = 0;
	int totalGems = 0;
};
struct BallModeUi {
	BallComponentOptions ballMode;
	std::optional<BallLevelSelectInfo> levelSelect;
};

struct MainMenu2Options {
	glm::vec4 backgroundColor;
	float offsetY;

	std::function<void()> onNewGame = []() -> void {};
	std::function<void()> onContinueGame = []() -> void {};
};

struct BallInfo {

};
struct LiveMenu {
	MainMenu2Options options;
	std::optional<BallInfo> ballInfo;

	std::string text;
};
struct GameOverUi {};

typedef std::variant<UiModeNone, FpsModeUi, BallModeUi, LiveMenu, GameOverUi> UiMode;
void changeUiMode(UiMode);

std::optional<BallModeUi*> getBallModeUI();
std::optional<LiveMenu*> getLiveMenuUi();

void setTerminalConfig(std::optional<TerminalConfig> terminalConfig);
std::optional<TerminalConfig*> getTerminalConfig();


extern Component playingComponent;

#endif

