#include "./orbs.h"

extern CustomApiBindings* gameapi;
extern OrbData orbData;

std::optional<Orb*> getOrb(std::vector<Orb>& orbs, int index){
	for (auto& orb : orbs){
		if (orb.index == index){
			return &orb;
		}
	}
	return std::nullopt;
}

glm::vec3 getOrbPosition(OrbUi& orbUi, int index){
	glm::vec3 offset = gameapi -> getGameObjectPos(orbUi.id, true, "getOrbPosition pos");
	auto scale = gameapi -> getGameObjectScale(orbUi.id, "getOrbPosition scale");
	auto rotation = gameapi -> getGameObjectRotation(orbUi.id, true, "getOrbPosition rotn");
	auto orbFrom = getOrb(orbUi.orbs, index);
	modassert(orbFrom.has_value(), std::string("orbFrom does not exist: ") + std::to_string(index));
	auto position = orbFrom.value() -> position;
	return offset +  (rotation * (scale * position));
}

glm::quat getOrbRotation(OrbUi& orbUi, int index){
	auto orbFrom = getOrb(orbUi.orbs, index);
	modassert(orbFrom.has_value(), std::string("orbFrom does not exist: ") + std::to_string(index));

	auto rotation = gameapi -> getGameObjectRotation(orbUi.id, true, "getOrbPosition rotn") * orbFrom.value() -> rotation;
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
		}else{
			drawSphereVecGfx(orbPosition, 0.2f, glm::vec4(1.f, 0.f, 0.f, 1.f));
		}

		auto orbMeshId = orbMeshCache(orbData, orbUi.id, orb.index);
		if (orb.mesh.has_value() &&  !orbMeshId.has_value()){
  			GameobjAttributes attr { 
  				.attr = {
					{ "mesh", orb.mesh.value() },
  				} 
  			};
  			std::unordered_map<std::string, GameobjAttributes> submodelAttributes;
  			auto meshId = gameapi -> makeObjectAttr(gameapi -> listSceneId(ownerId), std::string("orb-mesh") + uniqueNameSuffix() , attr, submodelAttributes);
  			modassert(meshId.has_value(), "meshId was not created");
  			saveMeshToOrbCache(orbData, orbUi.id, orb.index, meshId.value());
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

		auto targetOrb = getOrb(orbUi.orbs, objView.targetIndex);
		modassert(targetOrb.has_value(), "handleOrbViews could not find target orb");
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
	}
}

int getMaxOrbIndex(OrbUi& orbUi){
	int maxIndex = 0;
	for (auto& orb : orbUi.orbs){
		if (orb.index > maxIndex){
			maxIndex = orb.index;
		}
	}
	return maxIndex;
}

std::optional<OrbUi*> getOrbUi(OrbData& orbData, objid id){
	for (auto& [orbId, orbView] : orbData.orbUis){
		if (orbId == id){
			return &orbView;
		}
	}
	return std::nullopt;
}
OrbSelection handleOrbControls(OrbData& orbData, int key, int action){
	OrbSelection orbSelection {};

	for (auto& [id, orbView] : orbData.orbViewsCameraToOrb){

		if (orbData.orbUis.find(orbView.orbId) == orbData.orbUis.end()){
			continue;
		}
		bool changedIndex = false;

		auto selectedOrb = getOrb(getOrbUi(orbData, orbView.orbId).value() -> orbs, orbView.targetIndex).value();
		auto isComplete = selectedOrb -> getOrbProgress().complete;

		if (isMoveLeftKey(key) && (action == 1)){
			orbView.targetIndex--;
			changedIndex = true;
			if (orbView.targetIndex < 0){
				orbView.targetIndex = 0;
				changedIndex = false;
			}
		}
		if (isMoveRightKey(key) && (action == 1) && isComplete){
			orbView.targetIndex++;
			changedIndex = true;
			auto orbUi = getOrbUi(orbData, orbView.orbId).value();
			auto maxIndex = getMaxOrbIndex(*orbUi);
			if(orbView.targetIndex > maxIndex){
				orbView.targetIndex = maxIndex;
				changedIndex = false;
			}
			
		}
		if (changedIndex){
			orbView.startTime = gameapi -> timeSeconds(false);
      		playGameplayClipById(getManagedSounds().activateSoundObjId.value(), std::nullopt, std::nullopt);
		}

		if (isJumpKey(key) && (action == 1)){
			auto selectedOrb = getOrb(getOrbUi(orbData, orbView.orbId).value() -> orbs, orbView.targetIndex);
			if (selectedOrb.has_value()){
				orbSelection.selectedOrb = selectedOrb;
	      		playGameplayClipById(getManagedSounds().activateSoundObjId.value(), std::nullopt, std::nullopt);
			}
		}

	}


	return orbSelection;
}

void setCameraToOrbView(objid cameraId, std::string orbUiName){
	std::optional<objid> orbId;
	for (auto &[id, orbUi] : orbData.orbUis){
		if (orbUi.name == orbUiName){
			orbId = id;
			break;
		}
	}
	modassert(orbId.has_value(), std::string("setCameraToOrbView no orbUi: ") + orbUiName);
	orbData.orbViewsCameraToOrb[cameraId] = OrbView {
		.orbId = orbId.value(),
		.actualIndex = 0,
		.targetIndex = 0,
		.startTime = std::nullopt,
	};
}
void removeCameraFromOrbView(objid cameraId){
  orbData.orbViewsCameraToOrb.erase(cameraId);
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