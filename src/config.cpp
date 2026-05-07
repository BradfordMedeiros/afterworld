#include "./config.h"

bool isLevelComplete(std::string);


std::vector<OrbUi> createOrbUi2(objid id, std::vector<OrbDataConfig>& orbDatas, std::vector<OrbDataConection>& orbConns){
	std::vector<OrbUi> orbUis;

	std::set<std::string> orbUiNames;
	for (auto& orbData : orbDatas){
		orbUiNames.insert(orbData.orbUi);
	}
	
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
	
			auto level = orbDatas.at(i).level;
			Orb orb {
				.index = i,
				.position = orbDatas.at(i).pos,
				.rotation = orbDatas.at(i).rotation,
				.tint = glm::vec4(1.f, 0.f, 1.f, 1.f),
				.mesh = std::nullopt,
				.level = level,
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
