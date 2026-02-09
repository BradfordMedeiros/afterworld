#include "./orbs.h"

extern CustomApiBindings* gameapi;
extern OrbData orbData;

bool showDebugInfo = true;

std::optional<Orb*> getOrb(std::vector<Orb>& orbs, int index){
	for (auto& orb : orbs){
		if (orb.index == index){
			return &orb;
		}
	}
	return std::nullopt;
}
std::vector<int> getAllConnections(OrbUi& orbUi, int index){
	std::set<int> connectsTo;
	for (int i = 0; i < orbUi.connections.size(); i++){
		auto& connection = orbUi.connections.at(i);
		if (connection.indexFrom == index){
			connectsTo.insert(connection.indexTo);
		}
		if (connection.indexTo == index){
			connectsTo.insert(connection.indexFrom);
		}
	}

	std::vector<int> allValues;
	for (auto id : connectsTo){
		allValues.push_back(id);
	}
	return allValues;
}

glm::vec3 getOrbPosition(OrbUi& orbUi, int index){
	glm::vec3 offset = gameapi -> getGameObjectPos(orbUi.ownerId, true, "getOrbPosition pos");
	auto scale = gameapi -> getGameObjectScale(orbUi.ownerId, "getOrbPosition scale");
	auto rotation = gameapi -> getGameObjectRotation(orbUi.ownerId, true, "getOrbPosition rotn");
	auto orbFrom = getOrb(orbUi.orbs, index);
	modassert(orbFrom.has_value(), std::string("orbFrom does not exist: ") + std::to_string(index));
	auto position = orbFrom.value() -> position;
	return offset +  (rotation * (scale * position));
}

glm::quat getOrbRotation(OrbUi& orbUi, int index){
	auto orbFrom = getOrb(orbUi.orbs, index);
	modassert(orbFrom.has_value(), std::string("orbFrom does not exist: ") + std::to_string(index));

	auto rotation = gameapi -> getGameObjectRotation(orbUi.ownerId, true, "getOrbPosition rotn") * orbFrom.value() -> rotation;
	return rotation;	
}

std::optional<objid> orbMeshCache(OrbData& orbData, objid orbId, int index){
	if (orbData.orbIdToIndexToMeshId.find(orbId) == orbData.orbIdToIndexToMeshId.end()){
		return std::nullopt;
	}
	auto& indexToObjId = orbData.orbIdToIndexToMeshId.at(orbId);
	if (indexToObjId.find(index) != indexToObjId.end()){
		return indexToObjId.at(index);
	}
	return std::nullopt;
}

void saveMeshToOrbCache(OrbData& orbData, objid orbId, int index, objid meshId){
	if (orbData.orbIdToIndexToMeshId.find(orbId) == orbData.orbIdToIndexToMeshId.end()){
		orbData.orbIdToIndexToMeshId[orbId] = {};
	}
	orbData.orbIdToIndexToMeshId.at(orbId)[index] = meshId;
}

void drawOrbs(OrbData& orbData, OrbUi& orbUi, int ownerId){
	for (auto& orb : orbUi.orbs){
		auto orbPosition = getOrbPosition(orbUi, orb.index);

		auto isComplete = orb.getOrbProgress().complete;
		if (isComplete){
			drawSphereVecGfx(orbPosition, 0.2f, orb.tint);
			drawSphereVecGfx(orbPosition, 0.1f, orb.tint);
		}else{
			drawSphereVecGfx(orbPosition, 0.2f, glm::vec4(1.f, 0.f, 0.f, 1.f));
		}

		auto orbMeshId = orbMeshCache(orbData, orbUi.ownerId, orb.index);
		if (orb.mesh.has_value() &&  !orbMeshId.has_value()){
  			GameobjAttributes attr { 
  				.attr = {
					{ "mesh", orb.mesh.value() },
  				} 
  			};
  			std::unordered_map<std::string, GameobjAttributes> submodelAttributes;
  			auto meshId = gameapi -> makeObjectAttr(gameapi -> listSceneId(ownerId), std::string("orb-mesh") + uniqueNameSuffix() , attr, submodelAttributes);
  			modassert(meshId.has_value(), "meshId was not created");
  			saveMeshToOrbCache(orbData, orbUi.ownerId, orb.index, meshId.value());
		}

		if (orbMeshId.has_value()){
			auto orbRotation = getOrbRotation(orbUi, orb.index);
			gameapi -> setGameObjectPosition(orbMeshId.value(), orbPosition, true, Hint { .hint = "[gamelogic] - orb set mesh posn" });
			gameapi -> setGameObjectRot(orbMeshId.value(), orbRotation, true, Hint { .hint = "[gamelogic] - orb set mesh rotn" });
		}
	}

	for (auto& connection : orbUi.connections){
		auto complete = getOrb(orbUi.orbs, connection.indexFrom).value() -> getOrbProgress().complete;
		if (!complete){
			continue;
		}

		auto orbFrom = getOrbPosition(orbUi, connection.indexFrom);
		auto orbTo = getOrbPosition(orbUi, connection.indexTo);
		gameapi -> drawLine(orbFrom, orbTo, false, ownerId, glm::vec4(0.f, 1.f, 0.f, 1.f), std::nullopt, std::nullopt);
	}
}

void handleOrbViews(OrbData& orbData){
	for (auto& [cameraId, objView] : orbData.orbViewsCameraToOrb){
		if (orbData.orbUis.find(objView.orbId) == orbData.orbUis.end()){
			continue;
		}
		OrbUi& orbUi = orbData.orbUis.at(objView.orbId);
		if (!objView.attachedToOrb){
			auto currTime = gameapi -> timeSeconds(false);
			auto elapsedTime = currTime - objView.moveToOrbStartTime.value();
			float percentage = elapsedTime / objView.duration.value();
			if (percentage > 1.f){
				objView.attachedToOrb = true;
			}else{
				auto targetOrbPosition = getOrbPosition(orbUi, objView.targetIndex);
				auto effectivePosition = lerp(objView.initialPosition.value(), targetOrbPosition, percentage);

				auto targetOrbRotation = getOrbRotation(orbUi, objView.targetIndex);
				auto effectiveRotation = glm::slerp(objView.initialRotation.value(), targetOrbRotation, percentage);

				gameapi -> setGameObjectPosition(cameraId, effectivePosition, true, Hint { .hint = "handleOrbViews set orb camera move to" });
				gameapi -> setGameObjectRot(cameraId, effectiveRotation, true, Hint { .hint = "handleOrbViews set orb camera rotn move to rot" });

				continue;
			}
		}

		auto targetOrb = getOrb(orbUi.orbs, objView.targetIndex);
		modassert(targetOrb.has_value(), std::string("handleOrbViews could not find target orb: ") + std::to_string(objView.targetIndex));
		if (targetOrb.value() -> image.has_value()){
			float opacity = objView.startTime.has_value() ? ((gameapi -> timeSeconds(false) - objView.startTime.value()) / 0.5f) : 1.f;
			if (opacity > 1.f) {
				opacity = 1.f;
			}
			//gameapi -> drawRect(-0.5f, 0.f, 0.5f, 0.5f, false, glm::vec4(1.f, 1.f, 1.f, 0.6f), std::nullopt, true, std::nullopt, targetOrb.value() -> image.value(), std::nullopt);
			gameapi -> drawRect(0.f, 0.f, 0.5f, 0.5f, false, glm::vec4(1.f, 1.f, 1.f, opacity), std::nullopt, true, std::nullopt, targetOrb.value() -> image.value(), std::nullopt);
		}
		if (targetOrb.value() -> text != ""){
			gameapi -> drawText(targetOrb.value() -> text, 0.f, 0.f, 8, false, std::nullopt, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
		}

		auto orbPosition = getOrbPosition(orbUi, objView.actualIndex);
		auto orbRotation = getOrbRotation(orbUi, objView.actualIndex);

		if (objView.startTime.has_value()){
			float elapsedAmount = gameapi -> timeSeconds(false) - objView.startTime.value();
			float percentage = elapsedAmount / 0.2f;

			auto targetOrbPosition = getOrbPosition(orbUi, objView.targetIndex);
			orbPosition = lerp(orbPosition, targetOrbPosition, percentage);

			auto targetOrbRotation = getOrbRotation(orbUi, objView.targetIndex);
			orbRotation = glm::slerp(orbRotation, targetOrbRotation, percentage);

			std::cout << "handleOrbViews: " << percentage << ", target = " << objView.targetIndex << ", actual = " << objView.actualIndex << std::endl;
			if (percentage > 1.f){
				orbPosition = targetOrbPosition;
				orbRotation = targetOrbRotation;
				objView.actualIndex = objView.targetIndex;
	      playGameplayClipById(getManagedSounds().activateSoundObjId.value(), std::nullopt, std::nullopt);
			}
		}

		std::cout << "handleOrbViews set position: " << gameapi -> getGameObjNameForId(cameraId).value() << ", to : " << print(orbPosition) << std::endl;
		std::cout << "handleOrbViews set rotn: " << print(orbRotation) << std::endl << std::endl;

		gameapi -> setGameObjectRot(cameraId, orbRotation, true, Hint { .hint = "handleOrbViews set orb camera rotn" });

		glm::vec3 cameraOffset = orbRotation * glm::vec3(0.f, 0.f, 2.f);
		gameapi -> setGameObjectPosition(cameraId, orbPosition + cameraOffset, true, Hint { .hint = "handleOrbViews set orb camera" });

		if (showDebugInfo){
			auto isComplete = targetOrb.value() -> getOrbProgress().complete;
			gameapi -> drawText(isComplete ? "complete" : "not complete", 0.f, 0.8f, 12, false, std::nullopt, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
		}

	}
}

int getMinOrbIndex(OrbUi& orbUi){
	int minIndex = orbUi.orbs.at(0).index;
	for (auto& orb : orbUi.orbs){
		if (orb.index < minIndex){
			minIndex = orb.index;
		}
	}
	return minIndex;
}

// These two should probably be based on the connections but in practice this is ok
int getPrevOrbIndex(OrbUi& orbUi, int targetIndex){
	auto connections = getAllConnections(orbUi, targetIndex);
	std::cout << "connections: from: " << targetIndex << ", conn = " << print(connections) << std::endl;
	auto newTargetIndex = targetIndex;
	for (int i = (connections.size() - 1); i >= 0; i--){
		auto id = connections.at(i);
		if (id < targetIndex){
			newTargetIndex = id;
			break;
		}
	}
	return newTargetIndex;
}

int getNextOrbIndex(OrbUi& orbUi, int targetIndex){
	auto connections = getAllConnections(orbUi, targetIndex);
	std::cout << "connections: from: " << targetIndex << ", conn = " << print(connections) << std::endl;
	auto newTargetIndex = targetIndex;
	for (auto id : connections){
		if (id > targetIndex){
			newTargetIndex = id;
			break;
		}
	}
	return newTargetIndex;
}
//////////////////////

std::optional<OrbUi*> getOrbUi(OrbData& orbData, objid id){
	for (auto& [orbId, orbView] : orbData.orbUis){
		if (orbId == id){
			return &orbView;
		}
	}
	return std::nullopt;
}

void setOrbSelectIndex(OrbView& orbView, int targetIndex){
	auto oldIndex = orbView.targetIndex;
	orbView.targetIndex = targetIndex;
	if (oldIndex != targetIndex){
		orbView.startTime = gameapi -> timeSeconds(false);
	  playGameplayClipById(getManagedSounds().activateSoundObjId.value(), std::nullopt, std::nullopt);
	}
}

std::vector<OrbSelection> handleOrbControls(OrbData& orbData, int key, int action){
	std::vector<OrbSelection> orbSelections;


	for (auto& [id, orbView] : orbData.orbViewsCameraToOrb){
		if (orbData.orbUis.find(orbView.orbId) == orbData.orbUis.end()){
			continue;
		}
		if (!orbView.attachedToOrb){
			continue;
		}

		OrbSelection orbSelection {};
		orbSelection.cameraId = id;

		auto orbUi = getOrbUi(orbData, orbView.orbId).value();
		orbSelection.orbView = &orbView;
		orbSelection.orbUi = orbUi;

		{
			if (isMoveLeftKey(key) && (action == 1)){
				auto connections = getAllConnections(*orbUi, orbView.targetIndex);
				if (connections.size() == 0 || orbView.targetIndex < connections.at(0) /* min index */){
					orbSelection.moveLeftNoSpace = true;
				}
				orbSelection.moveLeftKey = true;
			}
			if (isMoveRightKey(key) && (action == 1)){
				auto connections = getAllConnections(*orbUi, orbView.targetIndex);
				if (connections.size() == 0 || orbView.targetIndex > connections.at(connections.size() - 1) /* max index */){
					orbSelection.moveRightNoSpace = true;
				}
				orbSelection.moveRightKey = true;
			}
		}

		auto currentOrb = getOrb(getOrbUi(orbData, orbView.orbId).value() -> orbs, orbView.targetIndex);
		orbSelection.currentOrb = currentOrb;

		if (isJumpKey(key) && (action == 1)){
			orbSelection.selectKey = true;
		}

		if (key == 'O' && (action == 1)){
			orbSelection.optionKey = true;
		}
		orbSelections.push_back(orbSelection);
	}

	return orbSelections;
}

void setCameraToOrbView(objid cameraId, std::string orbUiName, std::optional<int> targetIndex, std::optional<float> time){
	for (auto &[id, orbUi] : orbData.orbUis){
		if (orbUi.name == orbUiName){
			orbData.orbViewsCameraToOrb[cameraId] = OrbView {
				.orbId = id,
				.actualIndex = targetIndex.has_value() ? targetIndex.value() : getMinOrbIndex(orbUi),
				.targetIndex = targetIndex.has_value() ? targetIndex.value() : getMinOrbIndex(orbUi),
				.startTime = std::nullopt,

				.attachedToOrb = time.has_value() ? false : true,
				.initialPosition = gameapi -> getGameObjectPos(cameraId, true, "[gamelogic] - setCameraToOrbView initial position"),
				.initialRotation = gameapi -> getGameObjectRotation(cameraId, true, "[gamelogic] - setCameraToOrbView initial rotation"),
				.moveToOrbStartTime = gameapi -> timeSeconds(false),
				.duration = time,
			};
			return;
		}
	}
	modassert(false, std::string("setCameraToOrbView no orbUi: ") + orbUiName);
}

void removeCameraFromOrbView(objid cameraId){
  orbData.orbViewsCameraToOrb.erase(cameraId);
}

std::optional<int> getMaxCompleteOrbIndex(OrbUi& orbUi){
	std::optional<int> maxCompleteIndex;
	for (auto& orb : orbUi.orbs){
		auto progress = orb.getOrbProgress();
		if (progress.complete){
			if (!maxCompleteIndex.has_value()){
				maxCompleteIndex = orb.index;
			}else if (orb.index > maxCompleteIndex.value()){
				maxCompleteIndex = orb.index;
			}
		}
	}
	return maxCompleteIndex;
}

std::optional<OrbUi*> orbUiByName(std::string orbUiName){
	for (auto &[_, orbUi] : orbData.orbUis){
		if (orbUi.name == orbUiName){
			return &orbUi;
		}
	}
	return std::nullopt;
}

std::optional<int> getActiveOrbViewIndex(objid cameraId){
	if (orbData.orbViewsCameraToOrb.find(cameraId) == orbData.orbViewsCameraToOrb.end()){
		return std::nullopt;
	}
	return orbData.orbViewsCameraToOrb.at(cameraId).targetIndex;
}

int numberOfOrbs(OrbView& orbView){
	auto orbId = orbView.orbId;
	auto& orbUi = orbData.orbUis.at(orbId);
	return orbUi.orbs.size();
}

std::optional<OrbView*> orbViewForCamera(objid cameraId){
	if (orbData.orbViewsCameraToOrb.find(cameraId) == orbData.orbViewsCameraToOrb.end()){
		return std::nullopt;
	}
	return &orbData.orbViewsCameraToOrb.at(cameraId);
}

std::optional<Orb*> selectedOrbForCamera(objid cameraId){
	auto orbView = orbViewForCamera(cameraId);
	if (!orbView.has_value()){
		return std::nullopt;
	}
	if (orbData.orbUis.find(orbView.value() -> orbId) == orbData.orbUis.end()){
		return std::nullopt;
	}
	OrbUi& orbUi = orbData.orbUis.at(orbView.value() -> orbId);

	auto targetIndex = orbView.value() -> targetIndex;
	for (auto& orb : orbUi.orbs){
		if (orb.index == targetIndex){
			return &orb;
		}
	}

	return std::nullopt;
}

std::string print(Orb& orb){
	std::string data;
	data += "index = [" + std::to_string(orb.index) + "] ";
	data += "tint =  [" + print(orb.tint) + "] ";
	data += "position = [" + print(orb.position) + "] ";
	data += "rotation = [ " + print(orb.rotation) + "] ";
	data += "level = [" + orb.level + "] ";

	return data;
}