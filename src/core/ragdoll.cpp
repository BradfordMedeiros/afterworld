#include "./ragdoll.h"

extern CustomApiBindings* gameapi;

bool isEntity(objid id);
void objectRemoved(objid idRemoved);

std::vector<BoneShape> boneValues = {
	BoneShape {
		.bone = "Hand",
		//.shape = PhysicsCreateSphere {
 		//	.radius = 0.5f,
		//},
		.shape = PhysicsCreateRect {
 			.width = 0.2f,
 			.height = 0.2f,
 			.depth = 0.2f,
		},
	},
	BoneShape {
		.bone = "Hips",
		//.shape = PhysicsCreateSphere {
 		//	.radius = 0.5f,
		//},
		.shape = PhysicsCreateRect {
 			.width = 0.2f,
 			.height = 0.2f,
 			.depth = 0.2f,
		},
	},
	BoneShape {
		.bone = "LeftUpLeg",
		//.shape = PhysicsCreateSphere {
 		//	.radius = 0.5f,
		//},
		.shape = PhysicsCreateRect {
 			.width = 0.2f,
 			.height = 0.2f,
 			.depth = 0.2f,
		},
	},
	BoneShape {
		.bone = "RightUpLeg",
		//.shape = PhysicsCreateSphere {
 		//	.radius = 0.5f,
		//},
		.shape = PhysicsCreateRect {
 			.width = 0.2f,
 			.height = 0.2f,
 			.depth = 0.2f,
		},
	},
	BoneShape {
		.bone = "Head",
		.shape = PhysicsCreateSphere {
 			.radius = 0.5f,
		},
	},
};


void createHitbox(objid playerModel){
	for (auto &value : boneValues){
		auto headValue = findChildObjBySuffix(playerModel, value.bone.c_str());
		if (headValue.has_value()){
			auto gameobj = gameapi -> getGameObjNameForId(headValue.value());
			std::cout << "debug createPhysicsBody: " << gameobj.value() << std::endl;

			gameapi -> createPhysicsBody(headValue.value(), value.shape, std::nullopt);
			rigidBodyOpts physicsOptions {
	  	  		.linear = glm::vec3(1.f, 1.f, 1.f),
	  	  		.angular = glm::vec3(0.f, 0.f, 0.f),
	  	  		.gravity = glm::vec3(0.f, -9.f, 0.f),
	  	  		.friction = 1.f,
	  	  		.restitution = 1.f,
	  	  		.mass = 1.f,
	  	  		.layer = physicsLayers.bones,
	  	  		.linearDamping = 0.f,
	  	  		.isStatic = true,
	  	  		.hasCollisions = false,
			};
			gameapi -> setPhysicsOptions(headValue.value(), physicsOptions);	
		}	
	}
}

std::optional<RigHit> handleRigHit(objid id) {
	auto groupId = gameapi -> groupId(id);
	auto isControllable = isEntity(groupId);
	if (!isControllable){
		return std::nullopt;
	}
	auto objectName = gameapi -> getGameObjNameForId(id).value();
	auto isHead = objectName.find("Head") != std::string::npos;
	std::cout << "doDamage: is head: " << (isHead ? "true" : "false") << std::endl;

	return RigHit {
		.isHeadShot = isHead,
		.mainId = groupId,
	};
}

std::string print(RigHit& righit){
	std::string value = "";
	value += std::string("headshot: ") + (righit.isHeadShot ? "true" : "false");
	return value;
}


std::optional<objid> findBodyPart(objid entityId, const char* part){
  auto children = gameapi -> getChildrenIdsAndParent(entityId);
  for (auto childId : children){
    auto name = gameapi -> getGameObjNameForId(childId).value();
    if (stringEndsWith(name, part)){
      return childId;
    }
  } 
  return std::nullopt;
}

std::vector<objid> findBodyPartAndChildren(objid entityId, const char* part){
  auto bodyPart = findBodyPart(entityId, part);
  if (!bodyPart.has_value()){
  	return {};
  }
  auto childIds = gameapi -> getChildrenIdsAndParent(bodyPart.value());
  std::vector<objid> ids;
  for (auto id : childIds){
  	ids.push_back(id);
  }
  return ids;
}

std::set<objid> entityIdsToDisable(objid entityId){
  std::set<objid> ids;
  auto leftHand = findBodyPart(entityId, "LeftHand");
  auto rightHand = findBodyPart(entityId, "RightHand");
  auto neck = findBodyPart(entityId, "Neck");
  auto head = findBodyPart(entityId, "Head");
  if (leftHand.has_value()){
	  ids.insert(leftHand.value());
  }
  if (rightHand.has_value()){
	  ids.insert(rightHand.value());
  }
  if (neck.has_value()){
	  ids.insert(neck.value());
  }
  if (head.has_value()){
	  ids.insert(head.value());
  }
  return ids;
}

std::set<objid> entityIdsToEnableForShooting(objid entityId){
	std::set<objid> ids;
	//{
  //	auto newIds = findBodyPartAndChilren(entityId, "RightShoulder");
  //	for (auto id : newIds){
  //		ids.insert(id);
  //	}
	//}
	//{
  //	auto newIds = findBodyPartAndChilren(entityId, "LeftShoulder");
  //	for (auto id : newIds){
  //		ids.insert(id);
  //	}
	//}
	{
  	auto newIds = findBodyPartAndChildren(entityId, "Spine");
  	for (auto id : newIds){
  		ids.insert(id);
  	}
	}
	return ids;
}

void enterRagdoll(objid playerModel){
	objectRemoved(playerModel);
	setGameObjectPhysicsEnable(playerModel, false);

	for (auto &value : boneValues){
		auto headValue = findChildObjBySuffix(playerModel, value.bone.c_str());
		auto gameobj = gameapi -> getGameObjNameForId(headValue.value());
		std::cout << "debug createPhysicsBody: " << gameobj.value() << std::endl;

		rigidBodyOpts physicsOptions {
		  .linear = glm::vec3(1.f, 1.f, 1.f),
		  .angular = glm::vec3(0.f, 0.f, 0.f),
		  .gravity = glm::vec3(0.f, -9.81f, 0.f),
		  .friction = 5.f,
		  .restitution = 1.f,
		  .mass = 1.f,
		  .layer = physicsLayers.bones,
		  .linearDamping = 0.f,
		  .isStatic = false,
		  .hasCollisions = true,
		};
		gameapi -> setPhysicsOptions(headValue.value(), physicsOptions);		
	}
}