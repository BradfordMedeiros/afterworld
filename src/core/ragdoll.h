#ifndef MOD_AFTERWORLD_RAGDOLL
#define MOD_AFTERWORLD_RAGDOLL

#include <iostream>
#include <vector>
#include "../../../ModEngine/src/cscript/cscript_binding.h"
#include "../util.h"
#include "../resources/layer.h"

struct BoneShape {
	std::string bone;
	ShapeCreateType shape;
};

extern std::vector<BoneShape> boneValues;

void createHitbox(objid playerModel);

struct RigHit {
	bool isHeadShot;
	objid mainId;
};

std::optional<RigHit> handleRigHit(objid id);
std::string print(RigHit& righit);



std::optional<objid> findBodyPart(objid entityId, const char* part);
std::vector<objid> findBodyPartAndChildren(objid entityId, const char* part);

std::set<objid> entityIdsToDisable(objid entityId);
std::set<objid> entityIdsToEnableForShooting(objid entityId);

#endif