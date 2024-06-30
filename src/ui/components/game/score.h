#ifndef MOD_AFTERWORLD_COMPONENTS_SCORE
#define MOD_AFTERWORLD_COMPONENTS_SCORE

#include "../common.h"
#include "../../../global.h"
#include "../basic/layout.h"
#include "../basic/listitem.h"

struct ScoreOptions {
	float timeRemaining;
	const char* gametypeName;
	int score1;
	int score2;
	int totalScore;
};

extern Component scoreComponent;

#endif

