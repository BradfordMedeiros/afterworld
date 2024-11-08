#include "./curves.h"

extern CustomApiBindings* gameapi;

std::unordered_map<objid, LinePoints> rails; 
std::unordered_map<objid, EntityOnRail> entityToRail;
std::unordered_map<objid, RaceData> entityToRaceData;

void drawCurve(LinePoints& line, glm::vec3 point, objid owner){
  for (int i = 0; i < (line.points.size() - 1); i++){
    auto pointFrom = line.points.at(i) + point;
    auto pointTo = line.points.at(i + 1) + point;
    gameapi -> drawLine(pointFrom, pointTo, false, owner, std::nullopt, std::nullopt, std::nullopt);
  }
}

std::optional<int> closestPointInCurve(LinePoints& linePoints, glm::vec3 point){
	std::optional<float> minDistance;
	std::optional<int> index;
	for (int i = 0; i < linePoints.points.size(); i++){
		auto distance = glm::distance(linePoints.points.at(i), point);
		if (!minDistance.has_value()){
			minDistance = distance;
			index = i;
		}else if (distance < minDistance.value()){
			minDistance = distance;
			index = i;
		}
	}
	return index;
}

struct CurvePoint {
  int lowIndex;
  int highIndex;
  float percentage;
  glm::vec3 point;
};

glm::vec3 calulateNextPointOnRail(LinePoints& linePoints, CurvePoint& curvePoint, float deltaAmount){
	auto lowPoint = linePoints.points.at(curvePoint.lowIndex);
	auto highPoint = linePoints.points.at(curvePoint.highIndex);
	auto alphaAmount = curvePoint.percentage + deltaAmount;
	if (alphaAmount > 1.f){
		alphaAmount = 1.f;
	}
	if (alphaAmount < 0.f){
		alphaAmount = 0.f;
	}
	auto value = glm::lerp(lowPoint, highPoint, alphaAmount);
	std::cout << "rail calulateNextPointOnRail, low = " << print(lowPoint) << ", high = " << print(highPoint) << ", target = " << print(value) << ", alpha = " << alphaAmount << std::endl;
	return value;
}


CurvePoint calculateProjectPoint(LinePoints& linePoints, int lowIndex, int highIndex, glm::vec3 point){
	modassert(highIndex < linePoints.points.size(), "calculateProjectPoint highIndex too high");
	auto point1 = linePoints.points.at(lowIndex);
	auto point2 = linePoints.points.at(highIndex);
	//std::cout << "rail lowIndex = " << lowIndex << ", highIndex = " << highIndex << std::endl;

	auto d1 = point2 - point1;
	auto d2 = point - point1;
	auto projectedOnToRailDistance 	= glm::dot(d1, d2);

	float percentage = projectedOnToRailDistance / (glm::length(d1) * glm::length(d2));
	if (percentage != percentage /* check for NaN */ ){
		percentage = 1.f; 
	}
	
	//std::cout << "rail = d1 = " << print(d1) << ", d2 = " << print(d2) << ", percentage = " << percentage << ", projectedOnToRailDistance = " << projectedOnToRailDistance <<  std::endl;
	auto newPoint = point1 + (glm::normalize(d1) * (percentage * glm::length(d2)));

	float percentageToNext = 1.f;
	auto distanceBetween = glm::distance(point1, point2);
	if (distanceBetween != 0.f){
		auto newDistance = glm::distance(newPoint, point1);
		percentageToNext = newDistance / distanceBetween;
		//std::cout << "rail = distanceBetween " <<  distanceBetween <<  ", new distance = " <<  newDistance << ", percentageToNext " << percentageToNext << ", p1 = " << print(newPoint) << ", p2 = " << print(point1) << std::endl;
		if (percentageToNext > 1.f){  // means you went beyond the point, kind of weird
			//std::cout << "rail went beyond" << std::endl;
			percentageToNext = 1.f;
		}		
	}

  return CurvePoint {
    .lowIndex = lowIndex,
    .highIndex = highIndex,
    .percentage = percentageToNext,
    .point = newPoint,
  };
}


struct LineSegment {
	int lowIndex;
	int highIndex;
};
struct LineSegmentToDistance {
	LineSegment lineSegment;
	float distance;
	glm::vec3 point;
};

std::vector<LineSegmentToDistance> calcSegmentForPoint(LinePoints& linePoints, glm::vec3 point){
	std::vector<LineSegmentToDistance> distances;
	for (int i = 0; i < (linePoints.points.size() - 1); i++){
		auto projectedPoint = calculateProjectPoint(linePoints, i, i+1, point);
		auto distanceToPoint = glm::distance(point, projectedPoint.point);
		distances.push_back(LineSegmentToDistance{
			.lineSegment = LineSegment {
				.lowIndex = i,
				.highIndex = (i + 1),
			},
			.distance = distanceToPoint,
			.point = projectedPoint.point,
		});
	}
	return distances;
}

float getMinDistance(std::vector<LineSegmentToDistance>& segments){
	float minDistance = segments.at(0).distance;
	for (int i = 1; i < segments.size(); i++){
		auto distance = segments.at(i).distance; 
		if (distance < minDistance){
			minDistance = distance;
		}
	}
	return minDistance;
}

bool isCornerPiece(std::vector<int>& minValues, std::vector<LineSegmentToDistance>& segments){
	bool foundFirst = false;
	bool foundEnd = false;

	for (auto minValue : minValues){
		if (segments.at(minValue).lineSegment.highIndex == (segments.size())){
			foundEnd = true;
		}
		if (segments.at(minValue).lineSegment.lowIndex == 0){
			foundFirst = true;
		}
	}
	return foundFirst && foundEnd;
}
LineSegment& pickSegment(std::vector<LineSegmentToDistance>& segments, bool direction){	
	modassert(segments.size() > 0, "no segment to pick");
	std::vector<int> minValues;
	float minDistance = getMinDistance(segments);
	for (int i = 0; i < segments.size(); i++){
		LineSegmentToDistance& segment = segments.at(i);
		if (segment.distance <= (minDistance + 0.1f)){
			minValues.push_back(i);
		}
	}

	//std::cout << "seg direction: " << (direction ? "true" : "false") << std::endl;
	//std::cout << "seg min values: ";
	//for (auto minValue : minValues){
	//	LineSegmentToDistance& segment = segments.at(minValue);
	//	std::cout << segment.distance << "( low = " << segment.lineSegment.lowIndex << ", high = " << segment.lineSegment.highIndex << " ) ";
	//}
	//std::cout << std::endl;

	auto corner = isCornerPiece(minValues, segments);
	if (corner){
		std::cout << "seg this is a corner" << std::endl;
		if (direction){
			return segments.at(0).lineSegment;
		}else{
			return segments.at(segments.size() - 1).lineSegment;
		}
	}

	if (!direction){
		// pick decreasing index
		//std::cout << "seg picked low: " << minValues.at(0) << std::endl;
		return segments.at(minValues.at(0)).lineSegment;
	}
	// pick increasing index
	//std::cout << "seg picked high: " << minValues.at(minValues.size() - 1) << std::endl << std::endl;
	return segments.at(minValues.at(minValues.size() - 1)).lineSegment;
}

CurvePoint calculateCurvePoint(LinePoints& linePoints, glm::vec3 point, bool direction){
	auto closestSegment = calcSegmentForPoint(linePoints, point);
	LineSegment& lineSegment = pickSegment(closestSegment, direction);
	//std::cout << "seg minIndex: low = " << lineSegment.lowIndex << ", high = " << lineSegment.highIndex << std::endl;
	return calculateProjectPoint(linePoints, lineSegment.lowIndex, lineSegment.highIndex, point);
}

const float RAIL_DISTANCE_TOLERANCE = 0.1f;

bool pointOnRail(LinePoints& linePoints, glm::vec3 point){
	auto curvePoint = calculateCurvePoint(linePoints, point, true);
	auto actualDistance = glm::distance(curvePoint.point, point);
	std::cout << "rail: actual distance: " << actualDistance << std::endl;
	return actualDistance < RAIL_DISTANCE_TOLERANCE;
}

float getTotalDistanceForCurve(LinePoints& line){
	float totalDistance = 0.f;
	for (int i = 0; i < (line.points.size() - 1); i++){
		auto point1 = line.points.at(i);
		auto point2 = line.points.at(i + 1);
		auto distance = glm::distance(point1, point2);
		totalDistance += distance;
	}
	return totalDistance;
}
float distanceRemaining(LinePoints& line, int index, float percentageToNext){
	float remainingDistanceAlongCurve = 0.f;
	for (int i = index; i < (line.points.size() - 1); i++){
		//	return (index + percentageToNext) / static_cast<float>(line.points.size() - 1);
		auto point1 = line.points.at(i);
		auto point2 = line.points.at(i + 1);
		auto distance = glm::distance(point1, point2);
		if (i == index){
			remainingDistanceAlongCurve += (distance * (1.f - percentageToNext));
		}else{
			remainingDistanceAlongCurve += distance;
		}
	}
	return remainingDistanceAlongCurve;
}

void handleRace(RaceData& raceData, glm::vec3 position, LinePoints& line){
 	auto curvePoint = calculateCurvePoint(line, position, true);
 	raceData.currentPosition = curvePoint.lowIndex;
 	raceData.percentageToNext = curvePoint.percentage;
 	if (raceData.currentPosition > raceData.maxPosition){
 		raceData.maxPosition = raceData.currentPosition;
 	}
 	if (distanceRemaining(line, raceData.currentPosition, raceData.percentageToNext) <= 0.f){
 		raceData.complete = true;
 	}
}

void attachToCurve(objid entityId, objid railId, bool direction){
  entityToRail[entityId] = EntityOnRail {
  	.railId = railId,
  	.direction = direction,
  };
}
void unattachToCurve(objid entityId){
  entityToRail.erase(entityId);
}

void setDirectionCurve(objid entityId, bool direction){
	entityToRail.at(entityId).direction = direction;
}
bool isAttachedToCurve(objid entityId){
	return entityToRail.find(entityId) != entityToRail.end();
}
std::optional<bool> getDirectionCurve(objid entityId){
	if (!isAttachedToCurve(entityId)){
		return std::nullopt;
	}
	return entityToRail.at(entityId).direction;
}
void maybeReverseDirection(objid entityId){
  auto direction = getDirectionCurve(entityId);
  if (direction.has_value()){
    setDirectionCurve(entityId, !direction.value());
  }
}

void addToRace(objid entityId){
	entityToRaceData[entityId] = RaceData {
		.currentPosition = 0,
		.maxPosition = 0,
		.percentageToNext = 0.f,
		.complete = false,
	};
}
void removeFromRace(objid entityId){
	entityToRaceData.erase(entityId);
}

void generateMeshForRail(objid sceneId, LinePoints& linePoints){
	std::vector<glm::vec3> face {
		glm::vec3(-0.05f, 0.f, 0.f),
		glm::vec3(0.f, 0.05f, 0.f),
		glm::vec3(0.05f, 0.f, 0.f),
	};
	std::vector<glm::vec3> points {
		glm::vec3(0.f, 0.f, 0.f),
		glm::vec3(30.f, 0.f, 0.f),
	};

	std::string meshName = "rail-mesh0";
	gameapi -> generateMesh(face, linePoints.points, meshName);

  GameobjAttributes attr {
    .attr = {
			{ "mesh", meshName },
			{ "position", glm::vec3(0.f, -0.2f, 0.f) },
			{ "texture", "./res/textures/testgradient2.png" },
		}
  };
  std::map<std::string, GameobjAttributes> submodelAttributes;
  auto id = gameapi -> makeObjectAttr(sceneId, "generatedMesh", attr, submodelAttributes);
}

const bool DRAW_CURVES = false;
void handleEntitiesOnRails(objid ownerId, objid sceneId){
	static bool doOnce = true;
	if (doOnce){
		doOnce = false;

	  rails[0] = LinePoints {
  		.points = {
    		glm::vec3(0.f, -27.f, 0.f),
    		glm::vec3(50.f, -27.f, 0.f),
    		glm::vec3(55.f, -27.f, -10.f),
    		glm::vec3(55.f, -17.f, -60.f),
    		glm::vec3(30.f, -17.f, -60.f),
    		//glm::vec3(0.f, -27.f, 0.f),
  		},
		};

		generateMeshForRail(sceneId, rails.at(0));
	}

	if (DRAW_CURVES){
  	for (auto &[id, line] : rails){
  		drawCurve(line, glm::vec3(0.f, 0.f, 0.f), ownerId);
  	}		
	}

 	for (auto &[id, raceData] : entityToRaceData){
		static objid activeRaceRailId = 0;
  	auto position = gameapi -> getGameObjectPos(id, true);
	  handleRace(raceData, position, rails.at(activeRaceRailId));
  	auto distance = distanceRemaining(rails.at(activeRaceRailId), raceData.currentPosition, raceData.percentageToNext);
	  auto totalDistance = getTotalDistanceForCurve(rails.at(0));
  	//gameapi -> drawText(std::string("race: ") + std::to_string(distance), 0.f, 0.f, 10, false, std::nullopt, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
  	//gameapi -> drawText(std::string("percentage: ") + std::to_string(distance / totalDistance), 0.f, -0.1f, 10, false, std::nullopt, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
  	//if (raceData.complete){
  	//  gameapi -> drawText("race finished!", 0.f, -0.2f, 10, false, std::nullopt, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
  	//}
 	}

 	for (auto &[id, entityOnRail] : entityToRail){
 		float speed = 0.5f;
 		auto position = gameapi -> getGameObjectPos(id, true);

  	auto entityAlreadyOnRail = pointOnRail(rails.at(entityOnRail.railId), position);
  	auto curvePoint = calculateCurvePoint(rails.at(entityOnRail.railId), position, entityOnRail.direction);
  	
  	auto direction = entityOnRail.direction ? 1  : -1;
  	auto nextPoint = calulateNextPointOnRail(rails.at(entityOnRail.railId), curvePoint, direction * speed * gameapi -> timeElapsed());
  	gameapi -> setGameObjectPosition(id, nextPoint, true);
 	}

}

const float RAIL_GRACE_DISTANCE = 0.5f;
std::optional<NearbyRail> nearbyRail(glm::vec3 position, glm::vec3 forwardDir){
 	for (auto &[id, line] : rails){
		auto points = calcSegmentForPoint(rails.at(0), position);
		for (int i = 0; i < points.size(); i++){
			auto& point = points.at(i);
			if (point.distance < RAIL_GRACE_DISTANCE){
				auto lowPoint = line.points.at(point.lineSegment.lowIndex);
				auto highPoint = line.points.at(point.lineSegment.highIndex);
				auto railDir = highPoint - lowPoint;

				auto dir = glm::dot(forwardDir, railDir);
				std::cout << "seg: " << dir << std::endl;
				return NearbyRail {
					.id = id,
					.direction = dir >= 0.f,
				};
			}
		}
 	}
	return std::nullopt;
}