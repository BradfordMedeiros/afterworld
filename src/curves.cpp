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
// Find the closest points, then project the point onto the line defined by those two points
CurvePoint calculateCurvePoint(LinePoints& linePoints, glm::vec3 point){
	auto closestIndex = closestPointInCurve(linePoints, point).value();
	if (glm::distance(point, linePoints.points.at(closestIndex)) < 5.f){
		closestIndex++;
		std::cout << "rail should advance" << std::endl;
	}else{

	}
	if (closestIndex >= linePoints.points.size()){  // means it's finished
		return CurvePoint {
			.lowIndex = linePoints.points.size() - 1,
			.highIndex = linePoints.points.size() - 1,
			.percentage = 0.f,
			.point = linePoints.points.at(linePoints.points.size() - 1),
		};
	}

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

	// this is wrong,  Can't really gauge it by distance since the next point might be further away if eg segments are different sizes
	// i think i should just see if we're in some delta distance to the next point, and if so go to the next rail location
	bool useNextPoint = true;
	if (!rawPointMinusOne.has_value() && rawPointPlusOne.has_value()){
		useNextPoint = true;
	}else if (rawPointMinusOne.has_value() && !rawPointPlusOne.has_value()){
		useNextPoint = false;
	}else {
		auto distanceMinusOne = glm::distance(point, rawPointMinusOne.value());
		auto distancePlusOne = glm::distance(point, rawPointPlusOne.value());
		useNextPoint = distancePlusOne <= distanceMinusOne;
		std::cout << "rail distancePlusOne = " << distancePlusOne << ", distanceMinusOne = " << distanceMinusOne << std::endl;
	}



	auto lowIndex =  useNextPoint ? closestIndex : closestIndexMinusOne;
	auto highIndex = useNextPoint ? closestIndexPlusOne : closestIndex;
	auto point1 = linePoints.points.at(lowIndex);
	auto point2 = linePoints.points.at(highIndex);
	std::cout << "rail lowIndex = " << lowIndex << ", highIndex = " << highIndex << std::endl;

	auto d1 = point2 - point1;
	auto d2 = point - point1;
	auto projectedOnToRailDistance 	= glm::dot(d1, d2);

	float percentage = projectedOnToRailDistance / (glm::length(d1) * glm::length(d2));
	auto newPoint = point1 + (glm::normalize(d1) * (percentage * glm::length(d2)));

	auto percentageToNext = glm::distance(newPoint, point1) / glm::distance(point1, point2);
	if (percentageToNext > 1.f){  // means you went beyond the point, kind of weird
		percentageToNext = 1.f;
	}
  return CurvePoint {
    .lowIndex = lowIndex,
    .highIndex = highIndex,
    .percentage = percentageToNext,
    .point = newPoint,
  };
}

const float RAIL_DISTANCE_TOLERANCE = 0.1f;

bool pointOnRail(LinePoints& linePoints, glm::vec3 point){
	auto curvePoint = calculateCurvePoint(linePoints, point);
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
 	auto curvePoint = calculateCurvePoint(line, position);
 	raceData.currentPosition = curvePoint.lowIndex;
 	raceData.percentageToNext = curvePoint.percentage;
 	if (raceData.currentPosition > raceData.maxPosition){
 		raceData.maxPosition = raceData.currentPosition;
 	}
 	if (distanceRemaining(line, raceData.currentPosition, raceData.percentageToNext) <= 0.f){
 		raceData.complete = true;
 	}
}

void attachToCurve(objid entityId, objid railId){
  entityToRail[entityId] = EntityOnRail {
  	.railId = railId,
  	.direction = true,
  };
}
void unattachToCurve(objid entityId){
  entityToRail.erase(entityId);
}

void setDirectionCurve(objid entityId, bool direction){
	entityToRail.at(entityId).direction = direction;
}
std::optional<bool> getDirectionCurve(objid entityId){
	if (!isAttachedToCurve(entityId)){
		return std::nullopt;
	}
	return entityToRail.at(entityId).direction;
}
bool isAttachedToCurve(objid entityId){
	return entityToRail.find(entityId) != entityToRail.end();
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


void handleEntitiesOnRails(objid ownerId){
	static bool doOnce = true;
	if (doOnce){
		doOnce = false;

	  rails[0] = LinePoints {
  		.points = {
    		glm::vec3(0.f, -20.f, 0.f),
    		glm::vec3(50.f, -20.f, 0.f),
    		glm::vec3(50.f, -20.f, -60.f),
  		},
		};
	}

  for (auto &[id, line] : rails){
  	drawCurve(line, glm::vec3(0.f, 0.f, 0.f), ownerId);
  }

 	for (auto &[id, raceData] : entityToRaceData){
 		break;
		static objid activeRaceRailId = 0;
  	auto position = gameapi -> getGameObjectPos(id, true);
	  handleRace(raceData, position, rails.at(activeRaceRailId));
  	auto distance = distanceRemaining(rails.at(activeRaceRailId), raceData.currentPosition, raceData.percentageToNext);
	  auto totalDistance = getTotalDistanceForCurve(rails.at(0));
  	gameapi -> drawText(std::string("race: ") + std::to_string(distance), 0.f, 0.f, 10, false, std::nullopt, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
  	gameapi -> drawText(std::string("percentage: ") + std::to_string(distance / totalDistance), 0.f, -0.1f, 10, false, std::nullopt, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
  	if (raceData.complete){
  	  gameapi -> drawText("race finished!", 0.f, -0.2f, 10, false, std::nullopt, std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
  	}
 	}

 	for (auto &[id, entityOnRail] : entityToRail){
 		float speed = 0.5f;
 		auto position = gameapi -> getGameObjectPos(id, true);

  	auto entityAlreadyOnRail = pointOnRail(rails.at(entityOnRail.railId), position);
  	std::cout << "rail: onrail = " << (entityAlreadyOnRail ? "true" : "false") << std::endl;

  	auto curvePoint = calculateCurvePoint(rails.at(entityOnRail.railId), position);
  	std::cout << "rail: low = " << curvePoint.lowIndex << ", high = " << curvePoint.highIndex << ", percentage = " << curvePoint.percentage << std::endl;
  	
  	auto direction = entityOnRail.direction ? 1  : -1;
  	auto nextPoint = calulateNextPointOnRail(rails.at(entityOnRail.railId), curvePoint, direction * speed * gameapi -> timeElapsed());
  	gameapi -> setGameObjectPosition(id, nextPoint, true);

 	}

}