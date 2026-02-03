#ifndef MOD_AFTERWORLD_CONFIG
#define MOD_AFTERWORLD_CONFIG

#include "./orbs.h"

OrbUi createOrbUi(objid id);

struct OrbDataConfig {
	glm::vec3 pos;
	glm::quat rotation;
	std::string level;
	std::string orbUi;
};
struct OrbDataConection {
	OrbConnection connection;
	std::string orbUi;
};

std::vector<OrbUi> createOrbUi2(objid id, std::vector<OrbDataConfig>& orbDatas, std::vector<OrbDataConection>& orbConns);

#endif