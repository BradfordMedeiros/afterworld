#include "./config.h"

bool isLevelComplete(std::string);

std::vector<Orb> ballGameOrbs {
	Orb {
		.index = 0,
		.position = glm::vec3(0.f, 0.f, 0.f),
		.rotation = MOD_ORIENTATION_FORWARD,
		.tint = glm::vec4(1.f, 0.f, 1.f, 1.f),
		.text = "level 0\nVideo\nPress Action To Play",
		.mesh = "../gameresources/build/uncategorized/arcade.gltf",
		.level = "video",
		.image = "../gameresources/build/textures/creepguy.png",
		.getOrbProgress = []() -> OrbProgress {
			return OrbProgress {
				.complete = isLevelComplete("video"),
			};
		},
	},
	Orb {
		.index = 1,
		.position = glm::vec3(2.f, 0.f, 0.f),
		.rotation = MOD_ORIENTATION_FORWARD,
		.tint = glm::vec4(0.f, 0.f, 1.f, 1.f),
		.text = "level 1\nIntro\nPress Action To Play",
		.mesh = std::nullopt,
		.level = "intro",
		.image = "../gameresources/build/textures/creepydog.jpg",
		.getOrbProgress = []() -> OrbProgress {
			return OrbProgress {
				.complete = isLevelComplete("intro"),
			};
		},
	},
	Orb {
		.index = 2,
		.position = glm::vec3(2.f, 1.f, 0.f),
		.rotation = MOD_ORIENTATION_FORWARD,
		.tint = glm::vec4(0.f, 1.f, 1.f, 1.f),
		.text = "level 2\nBall Rollingl\nPress Action To Play",
		.mesh = "../gameresources/build/uncategorized/arcade.gltf",
		.level = "ball2",
		.getOrbProgress = []() -> OrbProgress {
			return OrbProgress {
				.complete = isLevelComplete("ball2"),
			};
		},
	},
	Orb {
		.index = 3,
		.position = glm::vec3(2.f, 1.f, -2.f),
		.rotation = MOD_ORIENTATION_FORWARD,
		.tint = glm::vec4(0.f, 1.f, 1.f, 1.f),
		.text = "level 3\nArena\nPress Action To Play",
		.mesh = std::nullopt,
		.level = "arena",
		.getOrbProgress = []() -> OrbProgress {
			return OrbProgress {
				.complete = isLevelComplete("arena"),
			};
		},
	},
};

std::vector<OrbConnection> ballGameConnections {
	OrbConnection {
		.indexFrom = 0,
		.indexTo = 2,
	},
	OrbConnection {
		.indexFrom = 0,
		.indexTo = 1,
	},
	OrbConnection {
		.indexFrom = 1,
		.indexTo = 2,
	},
	OrbConnection {
		.indexFrom = 0,
		.indexTo = 3,
	},
	OrbConnection {
		.indexFrom = 2,
		.indexTo = 3,
	},
};

OrbUi createOrbUi(objid id){
  OrbUi orbUi {
    .ownerId = id,
    .name = "testorb2",
    .orbs = ballGameOrbs,
    .connections = ballGameConnections,
  };
  return orbUi;	
}


struct OrbMappingValue {
	std::string text;
	std::string level;
};
std::unordered_map<std::string, OrbMappingValue> nameToOrbMapping {
	{ "one", OrbMappingValue { 
			.text = "this is dev",
			.level = "dev",
		}
	},
	{ "two", OrbMappingValue { 
			.text = "this is video",
			.level = "video",
		}
	},
	{ "three", OrbMappingValue { 
			.text = "this is video (again)",
			.level = "video",
		}
	},
	{ "testorb", OrbMappingValue { 
			.text = "this testorb",
			.level = "testorb",
		}
	},
	{ "testorb3", OrbMappingValue { 
			.text = "this testorb 3",
			.level = "testorb3",
		}
	},
};

std::set<std::string> allOrbUis(std::vector<OrbDataConfig>& orbDatas){
	std::set<std::string> values;
	for (auto& orbData : orbDatas){
		values.insert(orbData.orbUi);
	}
	return values;
}

std::vector<OrbUi> createOrbUi2(objid id, std::vector<OrbDataConfig>& orbDatas, std::vector<OrbDataConection>& orbConns){
	std::vector<OrbUi> orbUis;

	auto orbUiNames = allOrbUis(orbDatas);

	for (auto& orbUiName : orbUiNames){
		OrbUi orbUi {
			.ownerId = id,
			.name = orbUiName,
		};
		std::vector<Orb> ballGameOrbs;
		std::vector<OrbConnection> ballGameConnections;

		for (int i = 0; i < orbDatas.size(); i++){
			if (orbDatas.at(i).orbUi != orbUiName){
				continue;
			}
			modassert(nameToOrbMapping.find(orbDatas.at(i).level) != nameToOrbMapping.end(), "no mapping for orbui level");
			auto& orbMappingValue = nameToOrbMapping.at(orbDatas.at(i).level);
			auto level = orbMappingValue.level;
			Orb orb {
				.index = i,
				.position = orbDatas.at(i).pos,
				.rotation = orbDatas.at(i).rotation,
				.tint = glm::vec4(1.f, 0.f, 1.f, 1.f),
				.text = orbMappingValue.text,
				.mesh = std::nullopt,
				.level = orbMappingValue.level,
				.image = std::nullopt,
				.getOrbProgress = [level]() -> OrbProgress {
					return OrbProgress {
						.complete = isLevelComplete(level),
					};
				},
			};
			ballGameOrbs.push_back(orb);
		}

		orbUi.orbs = ballGameOrbs;

		std::vector<OrbConnection> connections;
		for (auto& orbConn : orbConns){
			if (orbConn.orbUi != orbUiName){
				continue;
			}
			connections.push_back(orbConn.connection);
		}
		orbUi.connections = connections;
	
		orbUis.push_back(orbUi);
	}

	return orbUis;
}
