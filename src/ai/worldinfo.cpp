#include "./worldinfo.h"

///////////////////////////////////////////////

void updateState(WorldInfo& worldInfo, int symbol, std::any value, std::set<int> tags, STATE_TYPE_HINT typeHint){
  for (auto &anyValue : worldInfo.anyValues){
    if (anyValue.symbol == symbol){
      anyValue.hint = typeHint;
      anyValue.value = value;
      return;
    }
  }
  worldInfo.anyValues.push_back(AnyState {
    .symbol = symbol,
    .tags = tags,
    .hint = typeHint,
    .value = value,
  });
}
std::optional<std::any> getState(WorldInfo& worldInfo, int symbol){
  for (auto &anyValue : worldInfo.anyValues){
    if (anyValue.symbol == symbol){
      return anyValue.value;
    }
  }
  return std::nullopt;
}

std::vector<AnyState*> getAnyStateRefByTag(WorldInfo& worldInfo, std::set<int> tags){
  std::vector<AnyState*> values = {};
  for (auto &anyState : worldInfo.anyValues){
    bool allTagsFound = true;
    for (auto tag : tags){
      bool stateHasTag = anyState.tags.find(tag) != anyState.tags.end();
      if (!stateHasTag){
        allTagsFound = false;
        break;
      }   
    }
    if (allTagsFound){
      values.push_back(&anyState);
    }
  }
  return values;
}
std::vector<std::any> getStateByTag(WorldInfo& worldInfo, std::set<int> tags){
  std::vector<std::any> values = {};
  for (auto anyState : getAnyStateRefByTag(worldInfo, tags)){
    values.push_back(anyState -> value);
  }
  return values;
}

void printWorldInfo(WorldInfo& worldInfo){
  std::cout << "world info: [" << std::endl;

  std::cout << "  any = [" << std::endl;
  for (auto &anyValue : worldInfo.anyValues){
    std::string anyValueAsStr = "[cannot print - no hint]";
    if (anyValue.hint == STATE_VEC3){
      anyValueAsStr = print(getAnyValue<glm::vec3>(anyValue.value));
    }else if (anyValue.hint == STATE_ENTITY_POSITION){
      EntityPosition entityPosition = getAnyValue<EntityPosition>(anyValue.value);
      anyValueAsStr = std::string("id = ") + std::to_string(entityPosition.id) + ", pos = " + print(entityPosition.position);
    }

    std::string tagsAsStr = "";
    for (auto tag : anyValue.tags){
      tagsAsStr = tagsAsStr + " " + nameForSymbol(tag);
    }

    std::cout << "    [" << nameForSymbol(anyValue.symbol) << ", " << anyValueAsStr << "]" << " - tags: [" << tagsAsStr << " ]" << std::endl;
  }
  std::cout << "  ]" << std::endl;

  std::cout << "]" << std::endl;
}
