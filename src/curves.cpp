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
CurvePoint calculateCurvePoint(LinePoints& linePoints, glm::vec3 point){
	// calculate the closest point
	// then the one above, then consider that line
	// project the point onto that line
	// calculate the percentage for that line
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

  return CurvePoint {
    .lowIndex = lowIndex,
    .highIndex = highIndex,
    .percentage = percentage,
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

  for (auto &[id, rail] : entityToRail){
  	auto position = gameapi -> getGameObjectPos(playerId, true);
  	auto curvePoint = calculateCurvePoint(rails.at(rail.railId), position);
  	std::cout << "curve point: " << curvePoint.lowIndex << ", " << curvePoint.highIndex << ", " << curvePoint.percentage << ", " << print(curvePoint.point) << std::endl;
    gameapi -> drawLine(position, curvePoint.point, true, ownerId, std::nullopt, std::nullopt, std::nullopt);

  }
}