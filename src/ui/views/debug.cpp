#include "./debug.h"

extern CustomApiBindings* gameapi;

Component debugComponent {
  .draw = [](DrawingTools& drawTools, Props& props) -> BoundingBox2D {
	  auto config = typeFromProps<DebugConfig>(props, valueSymbol);
    modassert(config, "debugConfig must be defined for debugComponent");

  	for (int i = 0; i < config -> data.size(); i++){
    	auto& row = config -> data.at(i);
    	std::string rowStr = "";
    	for (auto &col : row){
    		auto strVal = std::get_if<std::string>(&col);
    		auto debugItem = std::get_if<DebugItem>(&col);
    		modassert(strVal || debugItem, "invalid type to config");
    		if(strVal){
    			rowStr += *strVal + " ";
    		}else if (debugItem){
    			rowStr += debugItem -> text + " ";
    		}
    	}
	    gameapi -> drawText(rowStr, -0.9f, 0.9 + (i * -0.1), 8, false, std::nullopt, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt);
  	}
    return BoundingBox2D {
    	.x = 0,
    	.y = 0,
    	.width = 0.f,
    	.height = 0.f,
    };
  },
};
