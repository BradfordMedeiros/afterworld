#include "./worldinfo.h"

///////////////////////////////////////////////

void updateBoolState(WorldInfo& worldInfo, int symbol, bool value){
  for (auto &boolValue : worldInfo.boolValues){
    if (boolValue.stateInfo.symbol == symbol){
      boolValue.value = value;
      return;
    }
  }
  worldInfo.boolValues.push_back(BoolState {
    .stateInfo = StateInfo {
      .symbol = symbol,
      .tags = {},
      .data = {},
    },
    .value = value,
  });
}
void updateVec3State(WorldInfo& worldInfo, int symbol, glm::vec3 value, std::set<int> tags, std::any data){
  for (auto &vec3Value : worldInfo.vec3Values){
    if (vec3Value.stateInfo.symbol == symbol){
      vec3Value.value = value;
      vec3Value.stateInfo.tags = tags;
      vec3Value.stateInfo.data = data;
      return;
    }
  }
  worldInfo.vec3Values.push_back(Vec3State {
    .stateInfo = StateInfo {
      .symbol = symbol,
      .tags = tags,
      .data = data,
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

std::optional<Vec3State*> getVec3StateRef(WorldInfo& worldInfo, int symbol){
  for (auto &vec3Value : worldInfo.vec3Values){
    if (vec3Value.stateInfo.symbol == symbol){
      return &vec3Value;
    }
  }
  return std::nullopt;
}

std::vector<Vec3State*> getVec3StateRefByTag(WorldInfo& worldInfo, std::set<int> tags){
  std::vector<Vec3State*> vecs = {};
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
      vecs.push_back(&vecState);
    }
  }
  return vecs;
}

std::vector<glm::vec3> getVec3StateByTag(WorldInfo& worldInfo, std::set<int> tags){
	std::vector<glm::vec3> vecs = {};
	for (auto vecState : getVec3StateRefByTag(worldInfo, tags)){
		vecs.push_back(vecState -> value);
	}
	return vecs;
}



std::optional<bool> getBoolState(WorldInfo& worldInfo, int symbol){
  for (auto &boolValue : worldInfo.boolValues){
    if (boolValue.stateInfo.symbol == symbol){
      return boolValue.value;
    }
  }
  return std::nullopt;
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
