#include "./orbs.h"

extern CustomApiBindings* gameapi;

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
	auto rotation = gameapi -> getGameObjectRotation(orbUi.id, true, "getOrbPosition rotn");
	return rotation;	
}

void drawOrbs(OrbUi& orbUi, int ownerId){
	for (auto& orb : orbUi.orbs){
		auto orbPosition = getOrbPosition(orbUi, orb.index);
		drawSphereVecGfx(orbPosition, 0.2f, orb.tint);
	}
	for (auto& connection : orbUi.connections){
		auto orbFrom = getOrbPosition(orbUi, connection.indexFrom);
		auto orbTo = getOrbPosition(orbUi, connection.indexTo);
		gameapi -> drawLine(orbFrom, orbTo, false, ownerId, glm::vec4(0.f, 1.f, 0.f, 1.f), std::nullopt, std::nullopt);
	}
}

void handleOrbViews(OrbData& orbData){
	for (auto& [cameraId, objView] : orbData.orbViewsCameraToOrb){
		OrbUi& orbUi = orbData.orbUis.at(objView.orbId);

		auto targetOrb = getOrb(orbUi.orbs, objView.targetIndex);
		modassert(targetOrb.has_value(), "handleOrbViews could not find target orb");
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

			std::cout << "handleOrbViews: " << percentage << ", target = " << objView.targetIndex << ", actual = " << objView.actualIndex << std::endl;
			if (percentage > 1.f){
				orbPosition = targetOrbPosition;
				objView.actualIndex = objView.targetIndex;
				objView.startTime = std::nullopt;
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
		bool changedIndex = false;
		if (isMoveLeftKey(key) && (action == 1)){
			orbView.targetIndex--;
			changedIndex = true;
			if (orbView.targetIndex < 0){
				orbView.targetIndex = 0;
				changedIndex = false;
			}
		}
		if (isMoveRightKey(key) && (action == 1)){
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

std::string print(Orb& orb){
	std::string data;
	data += "index = [" + std::to_string(orb.index) + "] ";
	data += "tint =  [" + print(orb.tint) + "] ";
	data += "position = [" + print(orb.position) + "] ";
	data += "level = [" + orb.level + "] ";

	return data;
}