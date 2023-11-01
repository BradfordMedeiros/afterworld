#include "./worldinfo.h"

///////////////////////////////////////////////

void updateState(WorldInfo& worldInfo, int symbol, std::any value, std::set<int> tags, STATE_TYPE_HINT typeHint){
  for (auto &anyValue : worldInfo.anyValues){
    if (anyValue.stateInfo.symbol == symbol){
      anyValue.hint = typeHint;
      anyValue.value = value;
      return;
    }
  }
  worldInfo.anyValues.push_back(AnyState {
    .stateInfo = StateInfo {
      .symbol = symbol,
      .tags = tags,
      .data = {},
    },
    .hint = typeHint,
    .value = value,
  });
}
std::optional<std::any> getState(WorldInfo& worldInfo, int symbol){
  for (auto &anyValue : worldInfo.anyValues){
    if (anyValue.stateInfo.symbol == symbol){
      return anyValue.value;
    }
  }
  return std::nullopt;
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

template <typename T> 
T getAnyValue(AnyState& anyState){
  auto anyValuePtr = anycast<T>(anyState.value);
  modassert(anyValuePtr, "get any value, invalid type provided");
  return *anyValuePtr;
}

void printWorldInfo(WorldInfo& worldInfo){
  std::cout << "world info: [" << std::endl;

  std::cout << "  vec3 = [" << std::endl;
  for (auto &vec3Value : worldInfo.vec3Values){
    std::cout << "    [" << nameForSymbol(vec3Value.stateInfo.symbol) << ", " << print(vec3Value.value) << "]" << std::endl;
  }
  std::cout << "  ]" << std::endl;

  std::cout << "  any = [" << std::endl;
  for (auto &anyValue : worldInfo.anyValues){
    std::string anyValueAsStr = "[cannot print - no hint]";
    if (anyValue.hint == STATE_VEC3){
      anyValueAsStr = print(getAnyValue<glm::vec3>(anyValue));
    }
    std::cout << "    [" << nameForSymbol(anyValue.stateInfo.symbol) << ", " << anyValueAsStr << "]" << std::endl;
  }
  std::cout << "  ]" << std::endl;

  std::cout << "]" << std::endl;
}
