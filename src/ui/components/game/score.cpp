#include "./score.h"

struct ScoreOptions {
	float timeRemaining;
	const char* gametypeName;
	int score1;
	int score2;
	int totalScore;
};

ScoreOptions scoreOptions {
	.timeRemaining = 93.f,
	.gametypeName = "deathmatch",
	.score1 = 5,
	.score2 = 8,
	.totalScore = 15,
};


float width = 0.5f;
float height = 0.4f;

Component scoreComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
    drawTools.drawRect(0.f, 0.f, width, height, false, glm::vec4(0.2f, 0.2f, 0.2f, 0.9f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);

    drawTools.drawRect(0.f, -0.1f, width, height * 0.5f, false, glm::vec4(0.2f, 0.2f, 0.8f, 0.9f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
    drawTools.drawRect(0.f, 0.1f, width, height * 0.5f, false, glm::vec4(0.8f, 0.2f, 0.2f, 0.9f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
	  drawCenteredText(drawTools, scoreOptions.gametypeName, 0.f, 0.f, 0.04f, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt);


  	return BoundingBox2D {
      .x = 0.f,
      .y = 0.f,
      .width = 2.f,
      .height = 2.f,
    };
  },
};
