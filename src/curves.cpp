#include "./curves.h"

extern CustomApiBindings* gameapi;

void drawCurve(LinePoints& line, glm::vec3 point, objid owner){
  for (int i = 0; i < (line.points.size() - 1); i++){
    auto pointFrom = line.points.at(i) + point;
    auto pointTo = line.points.at(i + 1) + point;
    gameapi -> drawLine(pointFrom, pointTo, false, owner, std::nullopt, std::nullopt, std::nullopt);
  }
}

LinePoints linePoints {
  .points = {
    glm::vec3(0.f, -20.f, 0.f),
    glm::vec3(50.f, -20.f, 0.f),
    glm::vec3(50.f, -20.f, -50.f),
  },
};

void drawAllCurves(objid owner){
	drawCurve(linePoints, glm::vec3(0.f, 0.f, 0.f), owner);
}

struct CurvePoint {
  int lowIndex;
  int highIndex;
  float percentage;
  glm::vec3 point;
};

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

// Find the closest points, then project the point onto the line defined by those two points
CurvePoint calculateCurvePoint(LinePoints& linePoints, glm::vec3 point){
	auto closestIndex = closestPointInCurve(linePoints, point).value();
	auto closestIndexPlusOne = closestIndex + 1;
	auto closestIndexMinusOne = closestIndex - 1;

	std::optional<glm::vec3> rawPointPlusOne;
	if (closestIndexPlusOne < linePoints.points.size()){
		rawPointPlusOne = linePoints.points.at(closestIndexPlusOne);
	}
	std::optional<glm::vec3> rawPointMinusOne;
	if (closestIndexMinusOne >= 0){
		rawPointMinusOne = linePoints.points.at(closestIndexMinusOne);
	}

	if (!rawPointMinusOne.has_value() && !rawPointPlusOne.has_value()){
		modassert(false, "calculateCurvePoint single point line only invalid");
	}

	bool useNextPoint = true;
	if (!rawPointMinusOne.has_value() && rawPointPlusOne.has_value()){
		useNextPoint = true;
	}else if (rawPointMinusOne.has_value() && !rawPointPlusOne.has_value()){
		useNextPoint = false;
	}else {
		auto distanceMinusOne = glm::distance(point, rawPointMinusOne.value());
		auto distancePlusOne = glm::distance(point, rawPointPlusOne.value());
		useNextPoint = distancePlusOne <= distanceMinusOne;
	}

	auto lowIndex =  useNextPoint ? closestIndex : closestIndexMinusOne;
	auto highIndex = useNextPoint ? closestIndexPlusOne : closestIndex;
	auto point1 = linePoints.points.at(lowIndex);
	auto point2 = linePoints.points.at(highIndex);

	auto d1 = point2 - point1;
	auto d2 = point - point1;
	auto projectedOnToRailDistance 	= glm::dot(d1, d2);

	float percentage = projectedOnToRailDistance / (glm::length(d1) * glm::length(d2));
	auto newPoint = point1 + (glm::normalize(d1) * (percentage * glm::length(d2)));

	auto percentageToNext = glm::distance(newPoint, point1) / glm::distance(point1, point2);
	if (percentageToNext > 1){  // means you went beyond the point, kind of weird
		percentageToNext = 1.f;
	}
  return CurvePoint {
    .lowIndex = lowIndex,
    .highIndex = highIndex,
    .percentage = percentageToNext,
    .point = newPoint,
  };
}

struct EntityOnRail {
	objid railId;
	bool direction;
};

std::unordered_map<objid, LinePoints> rails {
	{ 0, linePoints },
}; 
std::unordered_map<objid, EntityOnRail> entityToRail;


struct RaceData {
	int currentPosition;
	int maxPosition;
	float percentageToNext;
};

RaceData raceData {
	.currentPosition = 0,
	.maxPosition = 0,
	.percentageToNext = 0.f,
};

float distanceRemaining(LinePoints& line, int index, float percentageToNext){
	float totalDistance = 0.f;
	for (int i = index; i < (line.points.size() - 1); i++){
		//	return (index + percentageToNext) / static_cast<float>(line.points.size() - 1);

		auto point1 = line.points.at(i);
		auto point2 = line.points.at(i + 1);
		auto distance = glm::distance(point1, point2);
		if (i == index){
			totalDistance += (distance * (1.f - percentageToNext));
		}else{
			totalDistance += distance;
		}
	}
	return totalDistance;
}
void handleRace(glm::vec3 position, LinePoints& line){
 	auto curvePoint = calculateCurvePoint(line, position);
 	raceData.currentPosition = curvePoint.lowIndex;
 	raceData.percentageToNext = curvePoint.percentage;
 	if (raceData.currentPosition > raceData.maxPosition){
 		raceData.maxPosition = raceData.currentPosition;
 	}
}

void attachToCurve(objid entityId, objid railId){
  entityToRail[entityId] = EntityOnRail {
  	.railId = railId,
  };
}

void unattachToCurve(objid entityId){
  entityToRail.erase(entityId);
}

void handleEntitiesOnRails(objid ownerId, objid playerId){
  drawAllCurves(ownerId);
  attachToCurve(playerId, 0);

 	auto position = gameapi -> getGameObjectPos(playerId, true);

  handleRace(position, rails.at(0));

  auto distance = distanceRemaining(rails.at(0), raceData.currentPosition, raceData.percentageToNext);
  std::cout << "race, checkpoint = " << raceData.maxPosition << ", distance = " << distance << ", next percentage: " << raceData.percentageToNext << std::endl;

  gameapi -> drawText(std::string("race: ") + std::to_string(distance), 0.f, 0.f, 10, false, std::nullopt, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);

  return;
  
  for (auto &[id, rail] : entityToRail){
  	auto curvePoint = calculateCurvePoint(rails.at(rail.railId), position);
  	std::cout << "curve point: " << curvePoint.lowIndex << ", " << curvePoint.highIndex << ", " << curvePoint.percentage << ", " << print(curvePoint.point) << std::endl;
    gameapi -> drawLine(position, curvePoint.point, true, ownerId, std::nullopt, std::nullopt, std::nullopt);

  }
}