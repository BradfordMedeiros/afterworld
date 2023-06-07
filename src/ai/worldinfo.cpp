#include "./worldinfo.h"

std::unordered_map<std::string, int> goalnameToInt;
int symbolIndex = -1;
int getSymbol(std::string name){
  if (goalnameToInt.find(name) != goalnameToInt.end()){
    return goalnameToInt.at(name);
  }
  symbolIndex++;
  goalnameToInt[name] = symbolIndex;
  return symbolIndex;
} 
std::string nameForSymbol(int symbol){
  for (auto &[name, symbolIndex] : goalnameToInt){
    if (symbolIndex == symbol){
      return name;
    }
  }
  modassert(false, "could not find symbol for: " + symbol);
  return "";
}

///////////////////////////////////////////////

void updateBoolState(WorldInfo& worldInfo, std::string name, bool value){
  for (auto &boolValue : worldInfo.boolValues){
    if (boolValue.stateInfo.symbol == getSymbol(name)){
      boolValue.value = value;
      return;
    }
  }
  worldInfo.boolValues.push_back(BoolState {
    .stateInfo = StateInfo {
      .symbol = getSymbol(name),
      .tags = {},
    },
    .value = value,
  });
}
void updateVec3State(WorldInfo& worldInfo, std::string name, glm::vec3 value, std::set<int> tags){
  for (auto &vec3Value : worldInfo.vec3Values){
    if (vec3Value.stateInfo.symbol == getSymbol(name)){
      vec3Value.value = value;
      vec3Value.stateInfo.tags = tags;
      return;
    }
  }
  worldInfo.vec3Values.push_back(Vec3State {
    .stateInfo = StateInfo {
      .symbol = getSymbol(name),
      .tags = tags,
    },
    .value = value,
  });
}

std::optional<glm::vec3> getVec3State(WorldInfo& worldInfo, int symbol){
  for (auto &vec3Value : worldInfo.vec3Values){
    if (vec3Value.stateInfo.symbol == symbol){
      return vec3Value.value;
    }
  }
  return std::nullopt;
}

std::vector<glm::vec3> getVec3StateByTag(WorldInfo& worldInfo, std::set<int> tags){
	std::vector<glm::vec3> vecs = {};
	for (auto &vecState : worldInfo.vec3Values){
		bool allTagsFound = true;
		for (auto tag : tags){
			bool stateHasTag = vecState.stateInfo.tags.find(tag) != vecState.stateInfo.tags.end();
			if (!stateHasTag){
				allTagsFound = false;
				break;
			}		
		}
		if (allTagsFound){
			vecs.push_back(vecState.value);
		}
	}
	return vecs;
}


void printWorldInfo(WorldInfo& worldInfo){
  std::cout << "world info: [" << std::endl;

  std::cout << "  bool = [" << std::endl;
  for (auto &boolValue : worldInfo.boolValues){
    std::cout << "    [" << nameForSymbol(boolValue.stateInfo.symbol) << ", " << print(boolValue.value) << "]" << std::endl;
  }
  std::cout << "  ]" << std::endl;

  std::cout << "  vec3 = [" << std::endl;
  for (auto &vec3Value : worldInfo.vec3Values){
    std::cout << "    [" << nameForSymbol(vec3Value.stateInfo.symbol) << ", " << print(vec3Value.value) << "]" << std::endl;
  }
  std::cout << "  ]" << std::endl;

  std::cout << "]" << std::endl;
}
