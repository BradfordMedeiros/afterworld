#include "./conditional_spawn.h"

extern CustomApiBindings* gameapi;

bool isDayTime(){
	return false;
}

std::vector<bool> playerGems = { false, true, false };
bool hasGem(int index){
	return playerGems.at(index);
}

void onAddConditionId(objid id, std::string& value){
	// if the condition is true, we spawn it, else we delete it 
	if (value == "daytime"){
		if (isDayTime()){
			return;
		}
	}else if (value == "gem"){
		auto gemIndex = static_cast<int>(getSingleFloatAttr(id, "gem").value());
		if (hasGem(gemIndex)){
			return;
		}
	}else {
		modassert(false, std::string("invalid condition: ") + value);
	}

	gameapi -> removeByGroupId(id);
	
}
