#include "./config.h"

bool isLevelComplete(std::string);

struct OrbMappingValue {
	std::string level;
};
std::unordered_map<std::string, OrbMappingValue> nameToOrbMapping {
	{ "one", OrbMappingValue { 
			.level = "dev",
		}
	},
	{ "two", OrbMappingValue { 
			.level = "video",
		}
	},
	{ "three", OrbMappingValue { 
			.level = "video",
		}
	},
	{ "testorb", OrbMappingValue { 
			.level = "testorb",
		}
	},
	{ "testorb3", OrbMappingValue { 
			.level = "testorb3",
		}
	},
	{ "w1", OrbMappingValue { 
			.level = "dev",
		}
	},
	{ "dev", OrbMappingValue { 
			.level = "dev",
		}
	},
	{ "video", OrbMappingValue { 
			.level = "dev",
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
			modassert(nameToOrbMapping.find(orbDatas.at(i).level) != nameToOrbMapping.end(), std::string("no mapping for orbui level: ") + orbDatas.at(i).level);
			auto& orbMappingValue = nameToOrbMapping.at(orbDatas.at(i).level);
			auto level = orbMappingValue.level;
			Orb orb {
				.index = i,
				.position = orbDatas.at(i).pos,
				.rotation = orbDatas.at(i).rotation,
				.tint = glm::vec4(1.f, 0.f, 1.f, 1.f),
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
