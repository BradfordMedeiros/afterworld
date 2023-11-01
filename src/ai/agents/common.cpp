#include "./common.h"

extern CustomApiBindings* gameapi;

float MAX_DETECT_DISTANCE = 100.f;

std::vector<IdAndPosition> checkVisibleTargets(WorldInfo& worldInfo, objid agentId){
  auto agentPosition = gameapi -> getGameObjectPos(agentId, true);
  auto hitobjectVal = gameapi -> contactTestShape(   // probably parameterize on size / shape, visualize
    agentPosition, 
    orientationFromPos(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f)), 
    glm::vec3(MAX_DETECT_DISTANCE, MAX_DETECT_DISTANCE, MAX_DETECT_DISTANCE)
  );

  // maybe consider doing a raycast after we have this list here

  std::set<objid> hitobjects;
  for (auto hitobject : hitobjectVal){
    hitobjects.insert(hitobject.id);
  }

  std::set<int> symbols = { getSymbol("target") };
  auto targetAttr = getSingleAttr(agentId, "agent-target");
  if (targetAttr.has_value()){
    symbols.insert(getSymbol(targetAttr.value()));
  }
  std::vector<IdAndPosition> idsAndPositions = {};
  auto targetPosVecStates = getVec3StateRefByTag(worldInfo, symbols);
  for (auto targetPosVecState : targetPosVecStates){
    TargetData* targetData = anycast<TargetData>(targetPosVecState -> stateInfo.data);
    modassert(targetData, "target data was null");
    if (hitobjects.count(targetData -> id) > 0){
      idsAndPositions.push_back(IdAndPosition {
        .id = targetData -> id,
        .position = targetPosVecState -> value,
      });
    }
  }

  return idsAndPositions;
}
