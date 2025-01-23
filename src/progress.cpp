#include "./progress.h"

GameProgress progress {
	.levels = LevelPlaylist {
		.currentProgress = 0,
		.shortNames = {
			"e1m1",
			"story-1",
			"e1m2",
			"story-end",
		},
	},
};

void setProgressByShortname(std::string name){
	for (int i = 0; i < progress.levels.shortNames.size(); i++){
		if(progress.levels.shortNames.at(i) == name){
			progress.levels.currentProgress = i;
			break;
		}
	}
}

std::optional<std::string> getCurrentLink(){
	if (!progress.levels.currentProgress.has_value()){
		return std::nullopt;
	}
	modassert(progress.levels.currentProgress.value() < progress.levels.shortNames.size(), "invalid current progress index too large");
	modassert(progress.levels.currentProgress.value() >= 0, "invalid current progress index to small");

	return progress.levels.shortNames.at(progress.levels.currentProgress.value());
}

void advanceProgress(){
	modassert(progress.levels.currentProgress.has_value(), "no current progress");
	progress.levels.currentProgress = progress.levels.currentProgress.value() + 1;
}

GameProgress& getGameProgress(){
	return progress;
}

