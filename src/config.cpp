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
    .id = id,
    .name = "testui",
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
			.text = "this is level one",
			.level = "dev",
		}
	},
	{ "two", OrbMappingValue { 
			.text = "this is level two",
			.level = "video",
		}
	},
	{ "three", OrbMappingValue { 
			.text = "this is level three",
			.level = "video",
		}
	},
};

OrbUi createOrbUi2(objid id, std::string name, std::vector<OrbDataConfig>& orbDatas, std::vector<OrbConnection>& orbConns){
	OrbUi orbUi {
		.id = id,
		.name = name,
	};
	std::vector<Orb> ballGameOrbs;
	std::vector<OrbConnection> ballGameConnections;

	for (int i = 0; i < orbDatas.size(); i++){
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
	orbUi.connections = orbConns;
	return orbUi;
}