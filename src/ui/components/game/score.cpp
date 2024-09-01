#include "./score.h"

float aspectRatio = 0.25f;
float width = 0.2f;
float height = width * aspectRatio;

struct ScoreConfig {
	float percentage;
	std::string score;
};

Component scoreBarComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
  	auto scoreConfigPtr = typeFromProps<ScoreConfig>(props, valueSymbol);
  	modassert(scoreConfigPtr, "score bar - no score config");

    float barWidth = width * scoreConfigPtr -> percentage;
    drawTools.drawRect(0.f, 0.f, width, height, false, glm::vec4(0.1f, 0.1f, 0.1f, 0.9f), true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
    drawTools.drawRect(0.f - (width * 0.5f) + (barWidth * 0.5f), 0.f, barWidth, height, false, glm::vec4(1.f, 1.f, 1.f, 0.4f), true, std::nullopt, "./res/textures/testgradient2.png", std::nullopt, std::nullopt);

		drawCenteredTextReal(drawTools, scoreConfigPtr -> score, 0.f, 0.f, 0.02f, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt);

  	return BoundingBox2D {
      .x = 0.f,
      .y = 0.f,
      .width = width,
      .height = height,
    };
  },
};

Component scoreComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
	 auto scoreOptionsPtr = typeFromProps<ScoreOptions>(props, valueSymbol);
	 modassert(scoreOptionsPtr, "scope options not provided to scoreComponent");
	 ScoreOptions& scoreOptions = *scoreOptionsPtr;

   bool negative = scoreOptions.timeRemaining < 0.f;
   float timeRemainingValue = scoreOptions.timeRemaining;
   if (negative){
    timeRemainingValue *= -1;
   }
	 int minutes = static_cast<int>(timeRemainingValue / 60.f);
	 int remainingSeconds = static_cast<int>(timeRemainingValue - (minutes * 60));
   std::string remainingSecondsStr = std::to_string(remainingSeconds);
   if (remainingSecondsStr.size() == 1){
      remainingSecondsStr = '0' + remainingSecondsStr;
   }
	 std::string remainingString = (negative ? std::string("-") : std::string("")) + std::to_string(minutes) + ":" + remainingSecondsStr;
   Props timeRemainingListItemProps {
      .props = {
        PropPair { .symbol = valueSymbol, .value = remainingString },
        PropPair { .symbol = fontsizeSymbol, .value = 0.02f },
        PropPair { .symbol = paddingSymbol, .value = 0.01f },
      },
    };
    auto timeRemaining = withProps(listItem, timeRemainingListItemProps);

    Props gametypeListItemProps {
      .props = {
        PropPair { .symbol = valueSymbol, .value = std::string(scoreOptions.gametypeName) },
        PropPair { .symbol = fontsizeSymbol, .value = 0.02f },
        PropPair { .symbol = paddingSymbol, .value = 0.01f },
        //PropPair { .symbol = tintSymbol, .value = glm::vec4(1.f, 0.f, 0.f, 0.8f) },
      },
    };
    auto gametypeTitle = withProps(listItem, gametypeListItemProps);

    Props scoreProps1 {
      .props = {
        PropPair { .symbol = valueSymbol, .value = ScoreConfig {
        	.percentage = static_cast<float>(scoreOptions.score1) / scoreOptions.totalScore,
        	.score = std::to_string(scoreOptions.score1),
        }},
      },
    };

    Props scoreProps2 {
      .props = {
        PropPair { .symbol = valueSymbol, .value = ScoreConfig {
        	.percentage = static_cast<float>(scoreOptions.score2) / scoreOptions.totalScore,
        	.score = std::to_string(scoreOptions.score2),
        }},
      },
    };

  	Layout layout {
  	  .tint = styles.primaryColor,
  	  .showBackpanel = true,
  	  .borderColor = styles.mainBorderColor,
  	  .minwidth = 0.f,
  	  .minheight = 0.f,
  	  .layoutType = LAYOUT_VERTICAL2,
  	  .layoutFlowHorizontal = UILayoutFlowNegative2,
  	  .layoutFlowVertical = UILayoutFlowPositive2,
  	  .alignHorizontal = UILayoutFlowNone2,
  	  .alignVertical = UILayoutFlowNone2,
  	  .spacing = 0.f,
  	  .minspacing = 0.f,
  	  .padding = 0.01f,
  	  .children = { 
  	  	timeRemaining, 
  	  	gametypeTitle, 
  	  	withProps(scoreBarComponent, scoreProps1), 
  	  	withProps(scoreBarComponent, scoreProps2) 
  	  },
  	};

  	Props listLayoutProps {
  	  .props = {
  	    { .symbol = layoutSymbol, .value = layout },
  	  },
  	};

  	auto xoffsetPtr = typeFromProps<float>(props, xoffsetSymbol);
  	auto yoffsetPtr = typeFromProps<float>(props, yoffsetSymbol);
  	if (xoffsetPtr){
  		listLayoutProps.props.push_back(PropPair { .symbol = xoffsetSymbol, .value = *xoffsetPtr });
  	}
  	if (yoffsetPtr){
  		listLayoutProps.props.push_back(PropPair { .symbol = yoffsetSymbol, .value = *yoffsetPtr });
  	}

  	return layoutComponent.draw(drawTools, listLayoutProps);

  },
};


