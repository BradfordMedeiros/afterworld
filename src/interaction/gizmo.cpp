#include "./gizmo.h"

extern CustomApiBindings* gameapi;
extern std::unordered_map<objid, GlassTexture> objIdToGlassTexture;
extern std::unordered_map<objid, Laser> lasers;
extern std::unordered_map<objid, GravityWell> gravityWells;
extern std::unordered_map<objid, TriggerColor> triggerColors;
extern std::unordered_map<objid, glm::vec3> impulses;
extern std::unordered_map<objid, LinkGunObj> linkGunObj;
extern std::unordered_map<objid, TeleportExit> teleportObjs;

//// glass //////////////////////////////////////////
void createGlassTexture(objid id){
	std::string textureName = std::string("glass-texture") + uniqueNameSuffix();
	auto glassTextureId = gameapi -> createTexture(textureName, 1000, 10000, id);
	objIdToGlassTexture[id] = GlassTexture {
		.id = glassTextureId,
		.name = textureName,
	};
    gameapi -> drawRect(0.f /*centerX*/, 0.f /*centerY*/, 2.f, 2.f, false, glm::vec4(1.f, 1.f, 1.f, 0.75f), glassTextureId, true, std::nullopt, "./res/textures/water.jpg", std::nullopt);
    setGameObjectTexture(id, objIdToGlassTexture.at(id).name);
}

void removeGlassTexture(objid id){
	auto textureName = objIdToGlassTexture.at(id).name;
	gameapi -> freeTexture(textureName, id);
	objIdToGlassTexture.erase(id);
}

// This should add normals, and drawRect should be able to subtractively add or something like that, so that
// can make the drawRect call subtract alpha
//
// This also only worked for glass the player is looking at...which is okay enough, but lame
// probably could do a quick render on  the object or something to map pos/normal of hit to uv space of object
bool maybeAddGlassBulletHole(objid id, objid playerId){
	if (objIdToGlassTexture.find(id) == objIdToGlassTexture.end()){
		return false;
	}
	GlassTexture& glassTexture = objIdToGlassTexture.at(id);
  	auto ndiCoord = uvToNdi(getGlobalState().texCoordUvView);
	float bulletHoleSize = randomNumber(0.f, 0.1f) + 0.1;

	 gameapi -> drawRect(ndiCoord.x /*centerX*/, ndiCoord.y /*centerY*/, bulletHoleSize, bulletHoleSize, false, glm::vec4(1.f, 1.f, 1.f, 0.8f), glassTexture.id, true, std::nullopt, "./res/textures/glassbroken.png", std::nullopt);
	return true;
}


//// laser ////////////////////////////////////////// 
void addLaser(objid id, float length){
	lasers[id] = Laser{};
	auto sceneId = gameapi -> listSceneId(id);
	{
		// the laser square is 5x5 (1x1 but then scale is 5 natively)
		float objectScale = gameapi -> getGameObjectScale(id, true).y;

		float width = 1.f;

		GameobjAttributes emitterAttr { 
  		.attr = {
 				{ "effekseer", "./res/particles/Laser02.efkefc" },
 				{ "state", "enabled" },
 				{ "killplane", "true" },
  		} 
  	};
  	std::unordered_map<std::string, GameobjAttributes> submodelAttributesEmitter;
  	auto laserParticle = gameapi -> makeObjectAttr(sceneId, std::string("+laser") + std::to_string(getUniqueObjId()), emitterAttr, submodelAttributesEmitter);

  	modassert(laserParticle.has_value(), "laserParticle was not created");
  	gameapi -> makeParent(laserParticle.value(), id);

  	// TODO HACK - this seems to be a bug where the emitter does not take the parent position
	simpleOnFrame([laserParticle, width, length, objectScale]() -> void {
	  gameapi -> setGameObjectScale(laserParticle.value(), glm::vec3(width, length, width), true);
	  gameapi -> setGameObjectPosition(laserParticle.value(), glm::vec3(0.f, (1.f / objectScale) * length * 0.5f, 0.f), false, Hint { .hint = "[gamelogic] - set laser pos" });
	}, 0.f);

	PhysicsCreateRect shape {
  		.width = 1.f,
  		.height = 1.f, 
  		.depth = 1.f,
	};
	auto offset = glm::vec3(0.f, 0.f, 0.f);
	gameapi -> createPhysicsBody(laserParticle.value(), shape, offset);
	}
}

void removeLaser(objid id){
	lasers.erase(id);
}

void onLaserFrame(){
	for (auto& [id, laser] : lasers){
  		auto fromPosition = gameapi -> getGameObjectPos(id, true, "[gamelogic] tags - laser");
 		auto rotation = gameapi -> getGameObjectRotation(id, true, "[gamelogic] - tags - laser");
 		auto toPos =  2.f * (rotation * glm::vec3(0.f, 0.f, -1.f));
 		gameapi -> drawLine(fromPosition, fromPosition + toPos, false, -1, std::nullopt,  std::nullopt, std::nullopt);
	}
}


//// gravityhole //////////////////////////////////////////
std::optional<GravityWell*> gravityWellByManaged(objid managed){
	for (auto& [id, gravityWell] : gravityWells){
		if (gravityWell.managedItem.has_value() && gravityWell.managedItem.value() == managed){
			return &gravityWell;
		}
	}
	return std::nullopt;
}
bool addToGravityWell(objid gravityWellId, objid managed){
	auto& gravityWell = gravityWells.at(gravityWellId);
	if (gravityWell.managedItem.has_value()){
		return false;
	}
	gravityWell.managedItem = managed;
	return true;
}
void removeFromGravityWell(objid managed){
	auto gravityWell = gravityWellByManaged(managed);
	if (gravityWell.has_value()){
		gravityWell.value() -> managedItem = std::nullopt;
	}
}

glm::vec3 getTargetWellPosition(GravityWell& gravityWell){
	auto wellPosition =  gameapi -> getGameObjectPos(gravityWell.id, true, "[gamelogic] wellPosition");
	auto wellRotation =  gameapi -> getGameObjectRotation(gravityWell.id, true, "[gamelogic] wellRotation");
  	auto targetWellPosition = wellPosition + (wellRotation * glm::vec3(0.f, 1.f, 0.f));
	return targetWellPosition;
}

void onFrameGravityWells(){
  	for (auto& [id, gravityWell] : gravityWells){
  		if (!gravityWell.managedItem.has_value()){
  			continue;
  		}
		auto position = gameapi -> getGameObjectPos(gravityWell.managedItem.value(), true, "[gamelogic] gravityhole");
		auto targetWellPosition = getTargetWellPosition(gravityWell);
		auto distance = targetWellPosition - position;
  		auto direction = glm::normalize(distance);
  		float speed = gameapi -> timeElapsed() * 10.f;
  		auto offset = glm::vec3(direction.x * speed, direction.y * speed, direction.z * speed);

  		// without this is will osscilate 
  		if ((offset.x > 0 && distance.x < offset.x) || (offset.x < 0 && distance.x > offset.x)){
  			offset.x = distance.x;
  		}
  		if ((offset.y > 0 &&  distance.y < offset.y) || (offset.y < 0 && distance.y > offset.y)){
  			offset.y = distance.y;
  		}
  		if ((offset.z > 0 && distance.z < offset.z) || (offset.z < 0 && distance.z > offset.z)){
  			offset.z = distance.z;
  		}
  		auto newPosition = position + offset;
		gameapi -> setGameObjectPosition(gravityWell.managedItem.value(), newPosition, false, Hint { .hint = "[gamelogic] - set well item posn" });
  	}
}

// Go to the closest gravity well in the direction specified by the direction
std::optional<glm::vec3> goToNextGravityWell(objid managed, glm::vec3 moveDirection){
	std::optional<glm::vec3> impulse;

	auto gravityWell = gravityWellByManaged(managed);

	std::optional<objid> newWellId;
	std::optional<float> newDistance;
	if (gravityWell.has_value()){
		auto wellFromPosition = gameapi -> getGameObjectPos(gravityWell.value() -> id, true, "[gamelogic] goToNextGravityWell closest well from");

		if (!gravityWell.value() -> launcher.has_value()){
			for (auto& [id, well] : gravityWells){
				if(gravityWell.value() -> id == well.id){
					continue;
				}

				if (gravityWell.value() -> target.has_value()){
					if (well.name.has_value() && (well.name.value() == gravityWell.value() -> target.value())){
						newWellId = id;
						break;
					}
				}else{
					auto wellToPos = gameapi -> getGameObjectPos(id, true, "[gamelogic] goToNextGravityWell closest well to");
					auto distance = glm::distance(wellFromPosition, wellToPos);
					auto direction = wellToPos - wellFromPosition;
					auto dir = glm::dot(direction, moveDirection);
					if (dir < 0){
						continue;
					}
					if (!newDistance.has_value()){
						newWellId = id;
						newDistance = distance;
					}else{
						if (distance < newDistance.value()){
							newDistance = distance;
							newWellId = id;
						}
					}					
				}

			}

			if (newWellId.has_value()){
				gravityWell.value() -> managedItem = std::nullopt;
				gravityWells.at(newWellId.value()).managedItem = managed;		
			}

		}else{
				gravityWell.value() -> managedItem = std::nullopt;
				auto launcherRot = gameapi -> getGameObjectRotation(gravityWell.value() -> id, true, "[gamelogic] - gravity well launcher");
				impulse = launcherRot * gravityWell.value() -> launcher.value();
		}	

	}
	return impulse;
}

bool shouldAutolaunchGravityWell(objid managed){
	auto gravityWellPtr = gravityWellByManaged(managed);
	if (!gravityWellPtr.has_value()){
		return false;
	}

	auto& gravityWell = *gravityWellByManaged(managed).value();
	float currTime = gameapi -> timeSeconds(false);
	if (!gravityWell.autolaunch){
		return false;
 	}

	auto wellPosition = getTargetWellPosition(gravityWell);
	auto managedPosition = gameapi -> getGameObjectPos(managed, true, "[gamelogic] gravity well managed pos");
	auto distance = glm::distance(wellPosition, managedPosition);
	if (distance < 0.1f){
		return true;
	}
	return false;
}

/////////////////
void triggerColor(std::string trigger){
	for (auto& [id, triggerColor] : triggerColors){
		if (!triggerColor.activeColor.has_value()){
			continue;
		}
		if (triggerColor.trigger == trigger){
			setGameObjectTint(id, triggerColor.activeColor.value());
		}
	}
}


void createExplosion(glm::vec3 position, float outerRadius, float damage){
	auto hitObjects = gameapi -> contactTestShape(position, glm::identity<glm::quat>(), glm::vec3(1.f * outerRadius, 1.f * outerRadius, 1.f * outerRadius));
	for (auto &hitobject : hitObjects){
		doDamageMessage(hitobject.id, damage);

		float force = 30.f;
		auto dirVec = glm::normalize(hitobject.point - position);
		applyImpulseAffectMovement(hitobject.id, glm::vec3(force * dirVec.x, force * dirVec.y, force * dirVec.z));
	}

	playGameplayClipById(getManagedSounds().explosionSoundObjId.value(), std::nullopt, position, false);
	auto activePlayer = getActivePlayerId(getDefaultPlayerIndex());
	if (activePlayer.has_value()){
		emitExplosion(rootSceneId(), activePlayer.value(), position, glm::vec3(1.f, 1.f, 1.f));
	}

	std::cout << "hitobjects: [";
	for (auto &hitobject : hitObjects){
		std::cout << gameapi -> getGameObjNameForId(hitobject.id).value() << " ";
	}
	std::cout << "]" << std::endl;
	simpleOnFrame([position, outerRadius]() -> void {
		drawSphereVecGfx(position, outerRadius, glm::vec4(0.f, 0.f, 1.f, 1.f));
	}, 0.2f);
}

void applyImpulseAffectMovement(objid id, glm::vec3 force){
  gameapi -> applyImpulse(id, force);
  impulses[id] = force;
}
std::optional<glm::vec3> getImpulseThisFrame(objid id){
  if (impulses.find(id) == impulses.end()){
    return std::nullopt;
  }
  return impulses.at(id);
}


void addLinkGunObj(objid id){
	linkGunObj[id] = LinkGunObj {};
}
void removeLinkGunObj(objid id){
  linkGunObj.erase(id);
  for (auto &[id, linkObj] : linkGunObj){
  	doDamageMessage(id, 1000.f);
  }
}
void onLinkGunObjFrame(){
  // check the nodes, if 
  for (auto &[id1, linkObj1] : linkGunObj){
	  for (auto &[id2, linkObj2] : linkGunObj){
  		if (id1 == id2){
  			continue;
  		}
			auto pos1 = gameapi -> getGameObjectPos(id1, true, "[gamelogic] tags - linkorb positions");
 			auto pos2 = gameapi -> getGameObjectPos(id2, true, "[gamelogic] tags - linkorb positions");
 			if (pos1.x < pos2.x){  // just so we only draw one connection between each, arbitrary function
  	 		gameapi -> drawLine(pos1, pos2, false, id1, glm::vec4(0.f, 0.f, 1.f, 1.f), std::nullopt, std::nullopt);
  	 	}
	  }
	}

	for (auto &[id, _] : linkGunObj){
		auto pos1 = gameapi -> getGameObjectPos(id, true, "[gamelogic] tags - link orb - vert lines");
		gameapi -> drawLine(pos1, pos1 + glm::vec3(0.f, 0.4f, 0.f), false, id, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);
	}
}

void handleTeleport(objid idToTeleport, objid teleporterId){
  auto teleportPosition = gameapi -> getGameObjectPos(teleporterId, true, "gamelogic get teleport position");
  gameapi -> setGameObjectPosition(idToTeleport, teleportPosition, true, Hint { .hint = "teleport set posn" });
  playGameplayClipById(getManagedSounds().teleportObjId.value(), std::nullopt, std::nullopt, false);
}

void doTeleport(int32_t idToTeleport, std::string destination){
  std::cout << "doTeleport: " << idToTeleport << ", = " << destination << std::endl;
  for (auto& [teleporterId, teleportExit] : teleportObjs){
    if (teleportExit.exit.has_value() && teleportExit.exit.value() == destination){
      handleTeleport(idToTeleport, teleporterId);
      std::cout << "doTeleport: found the exit" << std::endl;
      return;
    }
  }
}

std::optional<TeleportInfo> getTeleportPosition(){
	if (teleportObjs.size() == 0){
		return std::nullopt;
	}

	auto index = randomNumber(0, teleportObjs.size() - 1);
	int currIndex = 0;
	for(auto &[id, teleportExit] : teleportObjs){
		if (currIndex == index){
			auto position = gameapi -> getGameObjectPos(id, true, "[gamelogic] tags - getTeleportPosition");
			return TeleportInfo {
				.id = id,
				.position = position
			};
		}
		currIndex++;
	}
	return std::nullopt;
}
