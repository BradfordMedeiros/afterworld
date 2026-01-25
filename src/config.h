#ifndef MOD_AFTERWORLD_CONFIG
#define MOD_AFTERWORLD_CONFIG

#include "./orbs.h"

OrbUi createOrbUi(objid id);

struct OrbDataConfig {
	glm::vec3 pos;
	std::string level;
};

OrbUi createOrbUi2(objid id, std::string name, std::vector<OrbDataConfig>& orbDatas, std::vector<OrbConnection>& orbConns);


#endif